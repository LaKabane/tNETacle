/**
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
**/


#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined Windows
# define ssize_t SSIZE_T
#endif

#if defined Unix
# include <unistd.h>
#endif

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "networking.h"

#include "tnetacle.h"
#include "options.h"
#include "mc.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"
#include "hexdump.h"
#include "wincompat.h"

extern struct options serv_opts;

#if defined Windows
static void
send_buffer_to_device_thread(struct server *s, struct frame *frame)
{
    struct evbuffer *output = bufferevent_get_output(s->pipe_endpoint);

    /*No need to networkize the size, we are in local !*/
    evbuffer_add(output, &frame->size, sizeof frame->size);
    evbuffer_add(output, frame->frame, frame->size);
}
#endif

static void
forward_udp_frame_to_other_peers(struct server *s, struct frame *current_frame,
                                 struct sockaddr *current_sockaddr, unsigned int
                                 current_socklen) 
{
    struct mc *it = NULL;
    struct mc *ite = NULL;
    char name[INET6_ADDRSTRLEN];
    struct sockaddr_storage udp_storage;

    (void)current_socklen;
    for (it = v_mc_begin(&s->peers), ite = v_mc_end(&s->peers);
        it != ite;
        it = v_mc_next(it))
    {
        int err;
        struct sockaddr *udp_addr = (struct sockaddr *)&udp_storage;
        int socklen;

        /* If it's not the peer we received the data from. */
        if (evutil_sockaddr_cmp(current_sockaddr, it->p.address, 0) == 0)
            continue;
        memcpy(&udp_storage, it->p.address, it->p.len);
        /*
         * This section have to be rewriten to use a port number that've
         * been decided by the protocol.
         */
        /*{{{*/
        if (udp_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *sin = (struct sockaddr_in *)udp_addr;

            sin->sin_port = htons(7676);
            socklen = sizeof(sin);
        }
        else if (udp_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)udp_addr;

            sin6->sin6_port = htons(7676);
            socklen = sizeof(sin6);
        }
        /*}}}*/
        err = sendto(event_get_fd(s->udp.udp_endpoint),
                     current_frame->raw_packet,
                     current_frame->size + sizeof(struct packet_hdr), 0,
                     udp_addr, socklen);
        if (err == -1)
        {
            log_notice("error while sending to %s",
                       mc_presentation(it, name, sizeof name));
            break;
        }
        log_debug("adding %d bytes to %s's output buffer",
            current_frame->size, mc_presentation(it, name, sizeof name));
    }
}

static void
server_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = (struct server *)ctx;
    struct evbuffer *in = bufferevent_get_input(bev);

    /* Get rid of every input */
    (void)s;
    evbuffer_drain(in, evbuffer_get_length(in));
}

static int
_server_match_bev(struct mc const *a, struct mc const *b)
{
    return a->bev == b->bev;
}

static void
server_mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    struct server *s = (struct server *)ctx;

    (void)bev;
    (void)s;
    if (events & BEV_EVENT_CONNECTED)
    {
        struct mc *mc;
        struct mc tmp;

        /*
         * If we received the notification that the connection is established,
         * then we move the corresponding struct mc from s->pending_peers to
         * s->peers.
         */

        tmp.bev = bev;
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            log_info("connexion established.");
            memcpy(&tmp, mc, sizeof(tmp));
            v_mc_erase(&s->pending_peers, mc);
            v_mc_push(&s->peers, &tmp);
            mc_hello(&tmp);
        }
    }
    if (events & BEV_EVENT_ERROR)
    {
        struct mc *mc;
        struct mc tmp;
        int everr;
        int sslerr;

        tmp.bev = bev;
        everr = EVUTIL_SOCKET_ERROR();

        if (everr != 0)
        {
            log_warnx("Unexpected shutdown of the meta-connexion: (%d) %s",
                       everr, evutil_socket_error_to_string(everr));
        }
        while ((sslerr = bufferevent_get_openssl_error(bev)) != 0)
        {
            log_warnx("SSL error code (%d): %s in %s %s",
                       sslerr, ERR_reason_error_string(sslerr),
                       ERR_lib_error_string(sslerr),
                       ERR_func_error_string(sslerr));
        }
        /*
         * Find if the exception come from a pending peer or a
         * regular peer and close it.
         */
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            char name[128];

            mc_close(mc);
            log_debug("%s removed from the pending list",
                      mc_presentation(mc, name, sizeof name));
            v_mc_erase(&s->pending_peers, mc);
        }
        else
        {
            mc = v_mc_find_if(&s->peers, &tmp, _server_match_bev);
            if (mc != v_mc_end(&s->peers))
            {
                mc_close(mc);
                v_mc_erase(&s->peers, mc);
                log_debug("socket removed from the peer list");
            }
        }
    }
}

static void
listen_callback(struct evconnlistener *evl, evutil_socket_t fd,
                struct sockaddr *sock, int len, void *ctx)
{
    struct server *s = (struct server *)ctx;
    struct event_base *base = evconnlistener_get_base(evl);
    int errcode;
    struct mc mc;

    memset(&mc, 0, sizeof mc);
    /* Notifiy the mc_init that we are in an SSL_ACCEPTING state*/
    /* Even if we are not in a SSL context, mc_init know what to do anyway*/
    mc.ssl_flags = BUFFEREVENT_SSL_ACCEPTING;
    errcode = mc_init(&mc, base, fd, sock, (socklen_t)len, s->server_ctx);
    if (errcode != -1)
    {
        bufferevent_setcb(mc.bev, server_mc_read_cb, NULL,
                          server_mc_event_cb, s);
        log_debug("add the ssl client to the list");
        v_mc_push(&s->peers, &mc);
    }
    else
    {
        log_notice("Failed to init a meta connexion");
    }
}

static void
accept_error_cb(struct evconnlistener *evl, void *ctx)
{
    (void)evl;
    (void)ctx;
}


/*
 * On Windows, we use this function outside server.c for readability purpose.
 */
#if defined Windows
/*static*/ void
#else
static void
#endif
broadcast_udp_to_peers(struct server *s)
{
    struct frame *fit = v_frame_begin(&s->frames_to_send);
    struct frame *fite = v_frame_end(&s->frames_to_send);
    struct sockaddr_storage udp_addr;
    char name[512];

    /* For all the frames*/
    for (;fit != fite; fit = v_frame_next(fit))
    {
        struct mc *it = NULL;
        struct mc *ite = NULL;

        it = v_mc_begin(&s->peers);
        ite = v_mc_end(&s->peers);
        /* For all the peers*/
        for (;it != ite; it = v_mc_next(it))
        {
            int err;
            struct packet_hdr hdr;

            memset(&hdr, 0, sizeof (struct packet_hdr));
            memcpy(&udp_addr, it->p.address, it->p.len);
            /*
             * This section have to be rewriten to use a port number that've
             * been decided by the protocol.
             */
            /*{{{*/
            if (udp_addr.ss_family == AF_INET)
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)&udp_addr;

                sin->sin_port = htons(7676);
            }
            else if (udp_addr.ss_family == AF_INET6)
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&udp_addr;

                sin6->sin6_port = htons(7676);
            }
            /*}}}*/
            /* Header configuration for the packet */
            /* Convert the size to netword presentation*/
            hdr.size = htons(fit->size);
            /* Copy the header to the packet */
            /* Enought place have been allocated for header and the frame */
            memcpy(fit->raw_packet, &hdr, sizeof(hdr));
            err = sendto(event_get_fd(s->udp.udp_endpoint), fit->raw_packet,
                         fit->size + sizeof(struct packet_hdr), 0,
                         (struct sockaddr const *)&udp_addr, it->p.len);
            if (err == -1)
            {
                log_notice("error while sending to %s",
                           mc_presentation(it, name, sizeof name));
                break;
            }
            log_debug("udp adding %d(%-#2x) bytes to %s's output buffer",
                fit->size, fit->size,
                address_presentation((struct sockaddr *)&udp_addr, it->p.len,
                                     name, sizeof name));
        }
    }
}

static void
frame_free(struct frame const *f)
{
    free(f->raw_packet);
}

#if !defined Windows
static 
#endif
int
frame_alloc(struct frame *frame, unsigned int size)
{
    void *tmp_raw_packet = NULL;
    void *tmp_frame_ptr = NULL;

    /* Alloc the size of the whole packet, plus the size of the header */
    tmp_raw_packet = (void *)malloc(size
                                    + sizeof(struct packet_hdr));
    if (tmp_raw_packet == NULL)
    {
        return -1;
    }
    /* Shift the frame to pointer to just behind the header */
    tmp_frame_ptr = (void *)((intptr_t)tmp_raw_packet
                             + sizeof(struct packet_hdr));

    /* Commit the results */
    frame->raw_packet = tmp_raw_packet;
    frame->frame = tmp_frame_ptr;
    frame->size = size;
    return 0;
}

#if defined Windows
void
server_set_device(struct server *s, int fd)
{
    struct evconnlistener **it = NULL;
    struct evconnlistener **ite = NULL;

    it = v_evl_begin(&s->srv_list);
    ite = v_evl_end(&s->srv_list);
    /* Enable all the listeners */
    for (; it != ite; it = v_evl_next(it))
    {
        evconnlistener_enable(*it);
    }
    log_info("listeners started");
}
#else

static void
server_device_cb(evutil_socket_t device_fd, short events, void *ctx)
{
    struct server *s = (struct server *)ctx;

    if (events & EV_READ)
    {
        ssize_t n;
        struct frame tmp;

        /* I know it sucks. I'm waiting for libtuntap to handle*/
        /* a FIONREAD-like api.*/
#define FRAME_DYN_SIZE 1600
        frame_alloc(&tmp, FRAME_DYN_SIZE);
        while ((n = read(device_fd, tmp.frame, FRAME_DYN_SIZE)) > 0)
        {
            /* We cannot read more than a ushort, can we ? */
            tmp.size = (unsigned short)n;
            v_frame_push(&s->frames_to_send, &tmp);
            frame_alloc(&tmp, FRAME_DYN_SIZE);
        }
#undef FRAME_DYN_SIZE
        if (n == 0 || EVUTIL_SOCKET_ERROR() == EAGAIN) /* no errors occurs*/
        {
            broadcast_udp_to_peers(s);
            v_frame_foreach(&s->frames_to_send, frame_free);
            v_frame_clean(&s->frames_to_send);
        }
        else if (n == -1)
            log_warn("read on the device failed:");
        /* Don't forget to free the last allocated frame */
        /* As we are out of the loop, the last call to frame alloc is useless */
        frame_free(&tmp);
    }
}

void
server_set_device(struct server *s, int fd)
{
    struct evconnlistener **it = NULL;
    struct evconnlistener **ite = NULL;
    struct event_base *evbase = s->evbase;
    struct event *ev = event_new(evbase, fd, EV_READ | EV_PERSIST,
                                 server_device_cb, s);
    int err;

    err = evutil_make_socket_nonblocking(fd);
    if (err == -1)
        log_debug("fuck !");

    if (ev == NULL)
    {
        log_warn("failed to allocate the event handler for the device:");
        return;
    }
    s->device = ev;
    event_add(s->device, NULL);
    log_info("event handler for the device sucessfully configured");

    it = v_evl_begin(&s->srv_list);
    ite = v_evl_end(&s->srv_list);
    /* Enable all the listeners */
    for (; it != ite; it = v_evl_next(it))
    {
        evconnlistener_enable(*it);
    }
    log_info("listener started");
}

#endif

SSL_CTX *
evssl_init(void)
{
    SSL_CTX  *server_ctx;

    /* Initialize the OpenSSL library */
    SSL_load_error_strings();
    SSL_library_init();
    /* We MUST have entropy, or else there's no point to crypto. */
    if (!RAND_poll())
        return NULL;

    server_ctx = SSL_CTX_new(SSLv23_method());

    /* Load the certificate file. This is not needed now */
   /*! SSL_CTX_use_certificate_chain_file(server_ctx, "cert") ||*/
    if ((serv_opts.key_path != NULL && serv_opts.cert_path != NULL) &&
        (!SSL_CTX_use_certificate_chain_file(server_ctx, serv_opts.cert_path) ||
        !SSL_CTX_use_PrivateKey_file(server_ctx, serv_opts.key_path, SSL_FILETYPE_PEM)))
    {
        log_info("Couldn't read 'pkey' or 'cert' file.  To generate a key");
        log_info("and self-signed certificate, run:");
        log_info("  openssl genrsa -out pkey 2048");
        log_info("  openssl req -new -key pkey -out cert.req");
        log_info("  openssl x509 -req -days 365 -in cert.req -signkey pkey -out cert");
        return NULL;
    }
    return server_ctx;
}

static int
frame_recvfrom(int fd, struct frame *frame, struct sockaddr *saddr, unsigned int *socklen)
{
    int err = 0;
    struct packet_hdr hdr;
    unsigned short local_size;

    err = recvfrom(fd, (char *)&hdr, sizeof(struct packet_hdr),
        MSG_PEEK, saddr, socklen);
    /*
    ** Sometimes, on some OS, recvfrom return EMSGSIZE when the size of the
    ** peeked buffer is not enough to read the entire datagram.
    ** So we need to check for this specific case.
    */
    if (err == -1)
    {
        int local_err = EVUTIL_SOCKET_ERROR();

#if defined Windows
#define MSG_TOO_LONG WSAEMSGSIZE
#else
#define MSG_TOO_LONG EMSGSIZE
#endif
        if (local_err != MSG_TOO_LONG)
        {
            /* So there is a real error */
            return err;
        }
#undef MSG_TOO_LONG
    }
    local_size = ntohs(hdr.size);
    frame_alloc(frame, local_size);
    err = recv(fd, (char *)frame->raw_packet,
        frame->size + sizeof(struct packet_hdr), 0);
    return err;
}

static void
server_udp_cb(evutil_socket_t udp_fd, short event, void *ctx)
{
    struct server *s = (struct server *)ctx;
    struct frame current_frame;
    struct sockaddr_storage sockaddr;
    unsigned int socklen = sizeof sockaddr;
    int err;

    if (event & EV_READ)
    {
        memset(&current_frame, 0, sizeof current_frame);
        while((err = frame_recvfrom(udp_fd, &current_frame, (struct sockaddr *)&sockaddr, &socklen)) != -1)
        {
            log_debug("udp recv packet size=%d(%-#2x)", current_frame.size, current_frame.size);

            /* And forward it to anyone else but except current peer*/
            forward_udp_frame_to_other_peers(s, &current_frame,
                                             (struct sockaddr *)&sockaddr,
                                             socklen);
#if defined Windows
            /*
            * Send to current frame to the windows thread handling the tun/tap
            * devices and clean the evbuffer
            */
            send_buffer_to_device_thread(s, &current_frame);
#else
            /* Write the current frame on the device and clean the evbuffer*/
            write(event_get_fd(s->device), current_frame.frame, current_frame.size);
#endif
        }
    }
}

static int
server_init_udp(struct sockaddr *addr, int len)
{
    struct sockaddr_storage udp_addr;
    evutil_socket_t tmp_sock;
    int err;

    memcpy(&udp_addr, addr, len);
    tmp_sock = tnt_udp_socket(addr->sa_family);
    if (tmp_sock == -1)
        return -1;
    if (udp_addr.ss_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)&udp_addr;

        sin->sin_port = htons(7676);
    }
    else if (udp_addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&udp_addr;

        sin6->sin6_port = htons(7676);
    }
    err = bind(tmp_sock, (struct sockaddr *)&udp_addr, len);
    if (err == -1)
    {
        return -1;
    }
    err = evutil_make_socket_nonblocking(tmp_sock);
    if (err == -1)
    {
        return -1;
    }
    return tmp_sock;
}

int
server_init(struct server *s, struct event_base *evbase)
{
    struct cfg_sockaddress *it_listen = NULL;
    struct cfg_sockaddress *ite_listen = NULL;
    struct cfg_sockaddress *it_peer = NULL;
    struct cfg_sockaddress *ite_peer = NULL;
    evutil_socket_t udp_socket;
    int err;
    size_t i = 0;

    v_mc_init(&s->peers);
    v_mc_init(&s->pending_peers);
    v_frame_init(&s->frames_to_send);
    v_evl_init(&s->srv_list);
    s->evbase = evbase;

    it_listen = v_sockaddr_begin(&serv_opts.listen_addrs);
    ite_listen = v_sockaddr_end(&serv_opts.listen_addrs);
    /* Listen on all ListenAddress */
    for (; it_listen != ite_listen; it_listen = v_sockaddr_next(it_listen), ++i)
    {
        struct event *ev_udp = NULL;
        struct evconnlistener *evl = NULL;
        char listenname[INET6_ADDRSTRLEN];

        evl = evconnlistener_new_bind(evbase, listen_callback,
            s, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
            (struct sockaddr *)&it_listen->sockaddr, it_listen->len);
        if (evl == NULL) {
            log_warnx("Failed to allocate the listener to listen to %s",
                address_presentation((struct sockaddr *)&it_listen->sockaddr,
                it_listen->len, listenname, sizeof listenname));
             continue;
        }
        evconnlistener_set_error_cb(evl, accept_error_cb);
        evconnlistener_disable(evl);

        /* udp endpoint init */

        /*
         * We listen on the same address for the udp socket
         */
        udp_socket = server_init_udp((struct sockaddr *)&it_listen->sockaddr,
                                     it_listen->len);
        if (udp_socket == -1)
        {
            log_warnx("Failed to init the udp socket on %s",
                  address_presentation((struct sockaddr *)&it_listen->sockaddr,
                                           it_listen->len, listenname,
                                           sizeof listenname));
            continue;
        }
        ev_udp = event_new(evbase, udp_socket, EV_PERSIST | EV_READ,
                           server_udp_cb, s);
        if (ev_udp == NULL)
        {
            log_warnx("Failed to allocate the udp socket on %s",
                  address_presentation((struct sockaddr *)&it_listen->sockaddr,
                                           it_listen->len, listenname,
                                           sizeof listenname));
            continue;
        }
        s->udp.udp_endpoint = ev_udp;

        event_add(s->udp.udp_endpoint, NULL);

        v_evl_push(&s->srv_list, evl);
    }

    /* If we don't have any PeerAddress it's finished */
    if (serv_opts.peer_addrs.size == 0)
        return 0;

    it_peer = v_sockaddr_begin(&serv_opts.peer_addrs);
    ite_peer = v_sockaddr_end(&serv_opts.peer_addrs);
    for (;it_peer != ite_peer; it_peer = v_sockaddr_next(it_peer))
    {
        struct mc mc;
        char peername[INET6_ADDRSTRLEN];

        memset(&mc, 0, sizeof mc);
        address_presentation((struct sockaddr *)&it_peer->sockaddr,
            it_peer->len, peername, sizeof peername);
        /* Notifiy the mc_init that we are in an SSL_CONNECTING state*/
        /* Even if we are not in a SSL context, mc_init know what to do anyway*/
        mc.ssl_flags = BUFFEREVENT_SSL_CONNECTING;
        err = mc_init(&mc, evbase, -1, (struct sockaddr *)&it_peer->sockaddr,
                      it_peer->len, s->server_ctx);
        if (err == -1) {
            log_warn("unable to allocate a socket for connecting to %s", peername);
            continue;
        }
        bufferevent_setcb(mc.bev, server_mc_read_cb, NULL, server_mc_event_cb, s);
        err = bufferevent_socket_connect(mc.bev,
            (struct sockaddr *)&it_peer->sockaddr, it_peer->len);
        if (err == -1) {
            log_warn("unable to connect to %s", peername);
            continue;
        }
        v_mc_push(&s->pending_peers, &mc);
    }
    return 0;
}
