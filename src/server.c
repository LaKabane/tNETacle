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
#include "client.h"

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
forward_frame_to_other_peers(struct server *s, struct frame *current_frame, struct bufferevent *current_bev)
{
    struct mc *it = NULL;
    struct mc *ite = NULL;
    char name[INET6_ADDRSTRLEN];

    for (it = v_mc_begin(&s->peers), ite = v_mc_end(&s->peers);
        it != ite;
        it = v_mc_next(it))
    {
        int err;

        /* If its not the peer we received the data from. */
        if (it->bev == current_bev)
            continue;
        err = mc_add_frame(it, current_frame);
        if (err == -1)
        {
            log_notice("error while crafting the buffer to send to %p", it);
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
    ssize_t n;
    struct evbuffer *buf = NULL;
    struct frame current_frame;
    unsigned short size;
    unsigned short *network_size_ptr;

    memset(&current_frame, 0, sizeof current_frame);
    buf = bufferevent_get_input(bev);
    while (evbuffer_get_length(buf) != 0)
    {
        /*
         * Read and convert the first bytes of the buffer from network byte
         * order to host byte order.
         */
        network_size_ptr = (unsigned short *)evbuffer_pullup(buf, sizeof(size));
        size = ntohs(*network_size_ptr);

        /* We are going to drain 2 bytes just after, so we'd better count them.*/
        if (size > evbuffer_get_length(buf) - sizeof(size))
        {
            log_debug("receive an incomplete frame of %d(%-#2x) bytes but "
                "only %d bytes are available", size, *network_size_ptr,
                      evbuffer_get_length(buf));
            break;
        }
        log_debug("receive a frame of %d(%-#2x) bytes", size,
                   *network_size_ptr);
        evbuffer_drain(buf, sizeof(size));

        /* 
         * We fill the current_frame with the size in host byte order,
         * and the frame's data
         */
        current_frame.size = size;
        current_frame.frame = evbuffer_pullup(buf, size); 
        /* And forward it to anyone else but except current peer*/
        forward_frame_to_other_peers(s, &current_frame, bev);

#if defined Windows
        /*
         * Send to current frame to the windows thread handling the tun/tap
         * devices and clean the evbuffer
         */
        send_buffer_to_device_thread(s, &current_frame);
#else
        /* Write the current frame on the device and clean the evbuffer*/
        n = write(event_get_fd(s->device), current_frame.frame, current_frame.size);
#endif
        evbuffer_drain(buf, size);
    }
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
         * peer and close it.
         */
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            mc_close(mc);
            v_mc_erase(&s->pending_peers, mc);
            log_debug("socket removed from the pending list");
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
broadcast_to_peers(struct server *s)
{
    struct frame *fit = v_frame_begin(&s->frames_to_send);
    struct frame *fite = v_frame_end(&s->frames_to_send);
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

            err = mc_add_frame(it, fit);
            log_debug("adding %d(%-#2x) bytes to %s's output buffer",
                fit->size, fit->size, mc_presentation(it, name, sizeof name));
        }
    }
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
free_frame(struct frame const *f)
{
    free(f->frame);
}

static void
server_device_cb(evutil_socket_t device_fd, short events, void *ctx)
{
    struct server *s = (struct server *)ctx;

    if (events & EV_READ)
    {
        int _n = 0;
        ssize_t n;
        struct frame tmp;

        /* I know it sucks. I'm waiting for libtuntap to handle*/
        /* a FIONREAD-like api.*/
#define FRAME_DYN_SIZE 1600
        tmp.frame = (char *)malloc(FRAME_DYN_SIZE); /*XXX*/
        while ((n = read(device_fd, tmp.frame, FRAME_DYN_SIZE)) > 0)
        {
            tmp.size = (unsigned short)n;/* We cannot read more than a ushort*/
            v_frame_push(&s->frames_to_send, &tmp);
            //log_debug("Read a new frame of %d bytes.", n);
            tmp.frame = (char *)malloc(FRAME_DYN_SIZE); /*XXX*/
            _n++;
        }
        if (n == 0 || EVUTIL_SOCKET_ERROR() == EAGAIN) /* no errors occurs*/
        {
            //log_debug("Read %d frames in this pass.", _n);
            broadcast_to_peers(s);
            v_frame_foreach(&s->frames_to_send, free_frame);
            v_frame_clean(&s->frames_to_send);
        }
        else if (n == -1)
            log_warn("read on the device failed:");
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

static
void listen_client_callback(struct evconnlistener *evl, evutil_socket_t fd,
  struct sockaddr *sock, int len, void *ctx)
{
	struct server *s = (struct server *)ctx;
    struct event_base *base = evconnlistener_get_base(evl);
    int errcode;
    struct mc mc;

	log_debug("A client is connecting");
    memset(&mc, 0, sizeof mc);
    /* Notifiy the mc_init that we are in an SSL_ACCEPTING state*/
    /* Even if we are not in a SSL context, mc_init know what to do anyway*/
    mc.ssl_flags = BUFFEREVENT_SSL_ACCEPTING;
    errcode = mc_init(&mc, base, fd, sock, (socklen_t)len, s->server_ctx);
    if (errcode != -1)
    {
        bufferevent_setcb(mc.bev, client_mc_read_cb, NULL,
                          NULL, s);
        log_debug("client connected");
        //v_mc_push(&s->peers, &mc);
    }
    else
    {
        log_notice("Failed to init a meta connexion");
    }
}

int
server_init(struct server *s, struct event_base *evbase)
{
    struct cfg_sockaddress *it_listen = NULL;
    struct cfg_sockaddress *ite_listen = NULL;
    struct cfg_sockaddress *it_peer = NULL;
    struct cfg_sockaddress *ite_peer = NULL;
	struct evconnlistener *evl = NULL;
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
        char listenname[INET6_ADDRSTRLEN];

		evl = evconnlistener_new_bind(evbase, listen_callback,
		  s, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
		  (struct sockaddr *)&it_listen->sockaddr, it_listen->len);
		if (evl == NULL) {
			log_warnx("Failed to allocate the listener to listen to %s",
			  address_presentation((struct sockaddr *)
				&it_listen->sockaddr, it_listen->len, listenname,
				sizeof listenname));
			continue;
		}
		evconnlistener_set_error_cb(evl, accept_error_cb);
		evconnlistener_disable(evl);
		v_evl_push(&s->srv_list, evl);
	}
	// Listen enable for client in the ports registered
    it_listen = v_sockaddr_begin(&serv_opts.client_addrs);
    ite_listen = v_sockaddr_end(&serv_opts.client_addrs);
    /* Listen on all ListenAddress */
    for (; it_listen != ite_listen; it_listen = v_sockaddr_next(it_listen), ++i)
    {
        char listenname[INET6_ADDRSTRLEN];
		evl = evconnlistener_new_bind(evbase, listen_client_callback,
			  s, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
		  (struct sockaddr *)&it_listen->sockaddr, it_listen->len);
		if (evl == NULL) {
			log_warnx("Failed to allocate the listener to listen to %s",
			  address_presentation((struct sockaddr *)
				&it_listen->sockaddr, it_listen->len, listenname,
				sizeof listenname));
			continue;
		}
		evconnlistener_set_error_cb(evl, accept_error_cb);
		evconnlistener_enable(evl);
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
