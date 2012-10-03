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

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "networking.h"
#include "tnetacle.h"
#include "mc.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"
#include "wincompat.h"
#include "udp.h"
#include "frame.h"
#include "device.h"
#include "tntsched.h"
#include "options.h"

#define VECTOR_TYPE struct udp_peer
#define VECTOR_PREFIX udp
#define VECTOR_NON_STATIC
#include "vector.h"

extern struct options serv_opts;

static void
log_ssl_error(void)
{
    char errstr[150];
    unsigned long errval;

    errval = ERR_get_error();
    ERR_error_string_n(errval, errstr, sizeof(errstr));
    log_warnx("[DTLS] %s", errstr);
}

static SSL_CTX *
create_udp_ctx(void)
{
    int err;
    char const *key_path;
    char const *certfile_path;

    SSL_CTX *ctx = SSL_CTX_new(DTLSv1_method());
    if (ctx == NULL)
    {
        log_ssl_error();
        return NULL;
    }

    err = SSL_CTX_set_cipher_list(ctx, "HIGH:MEDIUM:aNULL");
    if (err != 1)
    {
        log_warnx("[DTLS] Unable to load ciphers, exiting");
        return NULL;
    }

    key_path = serv_opts.key_path;
    if (key_path == NULL)
    {
        log_warnx("[DTLS] Unable to load private key file");
        return NULL;
    }

    err = SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM);
    if (err == -1)
    {
        log_ssl_error();
        return NULL;
    }

    certfile_path = serv_opts.cert_path;
    if (certfile_path == NULL)
    {
        log_warnx("[DTLS] Unable to load certificate file");
        return NULL;
    }

    SSL_CTX_use_certificate_chain_file(ctx, certfile_path);
    return ctx;
}

void
forward_udp_frame_to_other_peers(struct udp *udp,
                                 struct frame *current_frame,
                                 struct sockaddr *current_sockaddr,
                                 unsigned int
                                 current_socklen) 
{
    struct udp_peer *it = NULL;
    struct udp_peer *ite = NULL;
    char name[INET6_ADDRSTRLEN];

    (void)current_socklen;
    for (it = v_udp_begin(udp->udp_peers), ite = v_udp_end(udp->udp_peers);
        it != ite;
        it = v_udp_next(it))
    {
        int err;

        /* If it's not the peer we received the data from. */
        if (evutil_sockaddr_cmp(current_sockaddr,
                                (struct sockaddr *)&it->addr, 0)
            == 0)
            continue;

        err = sendto(udp->fd,
                     current_frame->raw_packet,
                     current_frame->size + sizeof(struct packet_hdr), 0,
                     (struct sockaddr *)&it->addr,
                     it->socklen);
        if (err == -1)
        {
            log_notice("error while sending to %s",
                       address_presentation((struct sockaddr *)&it->addr,
                                            it->socklen,
                                            name,
                                            sizeof name));
            break;
        }
        log_debug("adding %d bytes to %s's output buffer",
            current_frame->size,
            address_presentation((struct sockaddr *)&it->addr,
                                 it->socklen,
                                 name,
                                 sizeof name));
    }
}

static int
dtls_new_peer(SSL_CTX *ctx, struct udp_peer *p)
{
    int err;
    SSL *ssl;

    err = BIO_new_bio_pair(&p->bio, UDP_MTU, &p->_bio_backend, UDP_MTU);
    if (err != 1)
    {
        log_ssl_error();
        ERR_print_errors_fp(stderr);
        return -1;
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL)
        return -1;

    if (p->ssl_flags & DTLS_CLIENT)
        SSL_set_connect_state(ssl);
    else
        SSL_set_accept_state(ssl);

    SSL_set_bio(ssl, p->_bio_backend, p->_bio_backend);
    if (ssl == NULL)
    {
        BIO_free(p->bio);
        BIO_free(p->_bio_backend);
        return -1;
    }
    p->ssl = ssl;
    return 0;
}

void
udp_register_new_peer(struct udp *udp,
                      struct sockaddr *s,
                      int socklen,
                      int ssl_flags)
{
    struct udp_peer tmp_udp;

    memcpy(&tmp_udp.addr, s, socklen);
    tmp_udp.socklen = socklen;
    tmp_udp.ssl_flags = ssl_flags;
    if (ssl_flags == DTLS_ENABLE)
        dtls_new_peer(udp->ctx, &tmp_udp);
    v_udp_push(udp->udp_peers, &tmp_udp);
}

void
broadcast_udp_to_peers(struct server *s)
{
    struct frame *fit = v_frame_begin(s->frames_to_send);
    struct frame *fite = v_frame_end(s->frames_to_send);
    struct udp   *udp = &s->udp;
    char name[INET6_ADDRSTRLEN];

    /* For all the frames*/
    for (;fit != fite; fit = v_frame_next(fit))
    {
        struct udp_peer *it = NULL;
        struct udp_peer *ite = NULL;

        it = v_udp_begin(udp->udp_peers);
        ite = v_udp_end(udp->udp_peers);
        /* For all the peers*/
        for (;it != ite; it = v_udp_next(it))
        {
            int err;
            struct packet_hdr hdr;

            memset(&hdr, 0, sizeof (struct packet_hdr));

            /* Header configuration for the packet */
            /* Convert the size to netword presentation*/
            hdr.size = htons(fit->size);
            /* Copy the header to the packet */
            /* Enought place have been allocated for header and the frame */
            memcpy(fit->raw_packet, &hdr, sizeof(hdr));
            err = sendto(udp->fd,
                         fit->raw_packet,
                         fit->size + sizeof(struct packet_hdr), 0,
                         (struct sockaddr const *)&it->addr,
                         it->socklen);
            if (err == -1)
            {
                log_notice("error while sending to %s",
                           address_presentation((struct sockaddr *)&it->addr,
                                                it->socklen,
                                                name,
                                                sizeof name));
                break;
            }
            log_debug("udp adding %d(%-#2x) bytes to %s's output buffer",
                fit->size, fit->size,
                address_presentation((struct sockaddr *)&it->addr,
                                     it->socklen,
                                     name,
                                     sizeof name));
        }
    }
}

void
server_udp(void *ctx)
{
    struct server *s = (struct server *)sched_get_userptr(ctx);
    struct frame current_frame;
    struct sockaddr_storage sockaddr;
    unsigned int socklen = sizeof sockaddr;
    evutil_socket_t udp_fd;
    int err;

    udp_fd = s->udp.fd;
    memset(&current_frame, 0, sizeof current_frame);
    while((err = frame_recvfrom(ctx,
                                udp_fd,
                                &current_frame,
                                (struct sockaddr *)&sockaddr,
                                &socklen)) != -1)
    {
        log_debug("udp recv packet size=%d(%-#2x)",
                  current_frame.size,
                  current_frame.size);

        /* And forward it to anyone else but except current peer*/
        forward_udp_frame_to_other_peers(&s->udp,
                                         &current_frame,
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


int
server_init_udp(struct server *s,
                struct sockaddr *addr,
                int len)
{
    int err;
    char name[INET6_ADDRSTRLEN];
    struct event_base *evbase = s->evbase;
    struct udp *udp = &s->udp;
    struct sockaddr_storage udp_addr;
    evutil_socket_t tmp_sock = 0;

    memcpy(&udp_addr, addr, len);
    tmp_sock = tnt_udp_socket(addr->sa_family);
    if (tmp_sock == -1)
        return -1;
    if (udp_addr.ss_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)&udp_addr;

        sin->sin_port = 0; /* 0 means random */
    }
    else if (udp_addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&udp_addr;

        sin6->sin6_port = 0; /* 0 means random */
    }
    err = bind(tmp_sock, (struct sockaddr *)&udp_addr, len);
    if (err == -1)
    {
        return -1;
    }
    getsockname(tmp_sock, (struct sockaddr *)&udp_addr, &len);
    log_debug("%s", address_presentation((struct sockaddr *)&udp_addr,
                                         len,
                                         name,
                                         sizeof name));
    err = evutil_make_socket_nonblocking(tmp_sock);
    if (err == -1)
    {
        return -1;
    }
    memcpy(&udp->udp_addr, &udp_addr, len);
    udp->udp_addrlen = len;
    udp->fd = tmp_sock;
    udp->udp_peers = v_udp_new();
    udp->udp_sched = sched_new(evbase);
    udp->udp_fiber = sched_new_fiber(udp->udp_sched, server_udp, (intptr_t)s);
    udp->ctx = create_udp_ctx();
    sched_fiber_launch(udp->udp_fiber);
    return 0;
}

int
frame_recvfrom(void *ctx,
               int fd,
               struct frame *frame,
               struct sockaddr *saddr,
               unsigned int *socklen)
{
    int err = 0;
    struct packet_hdr hdr;
    unsigned short local_size;

    err = async_recvfrom(ctx,
                         fd,
                         (char *)&hdr,
                         sizeof(struct packet_hdr),
                         MSG_PEEK,
                         saddr,
                         socklen);
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

unsigned short
udp_get_port(struct udp *udp)
{
    unsigned short port;

    if (udp->udp_addr.ss_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)&udp->udp_addr;

        port = ntohs(sin->sin_port);
    }
    else if (udp->udp_addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&udp->udp_addr;

        port = ntohs(sin6->sin6_port);
    }
    return port;
}

