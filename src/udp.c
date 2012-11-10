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
#include "subset.h"
#include "dtls.h"

void
forward_udp_frame_to_other_peers(void *async_ctx,
                                 struct udp *udp,
                                 struct frame *current_frame,
                                 struct sockaddr *current_sockaddr,
                                 unsigned int
                                 current_socklen) 
{
    struct udp_peer *it = NULL;
    struct udp_peer *ite = NULL;
    struct endpoint current_endp;

    endpoint_init(&current_endp, current_sockaddr, current_socklen);
    (void)current_socklen;
    for (it = v_udp_begin(udp->udp_peers), ite = v_udp_end(udp->udp_peers);
        it != ite;
        it = v_udp_next(it))
    {
        int err;

        /* If it's not the peer we received the data from. */
        if (endpoint_cmp(&it->peer_addr, &current_endp) == 0)
        {
            continue;
        }
        
        err = async_sendto(async_ctx,
                           udp->fd,
                           current_frame->raw_packet,
                           current_frame->size + sizeof(struct packet_hdr), 0,
                           endpoint_addr(&it->peer_addr),
                           endpoint_addrlen(&it->peer_addr));

        if (err == -1)
        {
            log_warn("[%s] error while sending to %s",
                     (it->ssl_flags & DTLS_ENABLE) ? "DTLS" : "UDP",
                     endpoint_presentation(&it->peer_addr));
            break;
        }
        log_debug("[%s] forwarding %d(%-#2x) bytes to %s",
                  (it->ssl_flags & DTLS_ENABLE) ? "DTLS" : "UDP",
                  current_frame->size, current_frame->size,
                  endpoint_presentation(&it->peer_addr));
    }
}

struct udp_peer *
udp_register_new_peer(struct udp *udp,
                      struct endpoint *remote,
                      int ssl_flags)
{
    struct udp_peer tmp_udp = {};
    struct endpoint *e = &tmp_udp.peer_addr;

    endpoint_copy(e, remote);
    tmp_udp.ssl_flags = ssl_flags;
    if (ssl_flags == DTLS_ENABLE)
    {
        int err = dtls_new_peer(udp->ctx, &tmp_udp);
        if (err == -1)
            log_warnx("[DTLS] failed to create ssl state for this peer");
    }
    log_info("[%s] peering with %s ",
             (ssl_flags & DTLS_ENABLE) ? "DTLS" : "UDP",
             endpoint_presentation(remote));
    return v_udp_insert(udp->udp_peers, &tmp_udp);
}

static void
_broadcast_udp_to_peers(struct server *s, void *async_ctx)
{
    struct frame *fit = v_frame_begin(s->frames_to_send);
    struct frame *fite = v_frame_end(s->frames_to_send);
    struct udp   *udp = s->udp;

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
            /* Enough space have been allocated for header and the frame */
            memcpy(fit->raw_packet, &hdr, sizeof(hdr));
            err = async_sendto(async_ctx,
                               udp->fd,
                               fit->raw_packet,
                               fit->size + sizeof(struct packet_hdr),
                               0,
                               endpoint_addr(&it->peer_addr),
                               endpoint_addrlen(&it->peer_addr));

            if (err == -1)
            {
                log_warn("[%s] error while sending to %s",
                         (it->ssl_flags & DTLS_ENABLE) ? "DTLS" : "UDP",
                         endpoint_presentation(&it->peer_addr));
                break;
            }
            log_debug("[%s] sending %d(%-#2x) bytes to %s",
                      (it->ssl_flags & DTLS_ENABLE) ? "DTLS" : "UDP",
                      fit->size, fit->size,
                      endpoint_presentation(&it->peer_addr));
        }
    }
    v_frame_foreach(s->frames_to_send, frame_free);
    v_frame_clean(s->frames_to_send);
}

void
broadcast_udp(void *ctx)
{
    struct server *s = (struct server *)sched_get_userptr(ctx);

    while (1)
    {
        async_yield(ctx, /*unused*/0);
        _broadcast_udp_to_peers(s, ctx);
    }
}

void
broadcast_udp_to_peers(struct server *s)
{
    struct fiber *F = s->udp->udp_brd_fib;

    async_wake(F, /*unused*/0);
}

void
server_udp(void *ctx)
{
    struct server *s = (struct server *)sched_get_userptr(ctx);
    struct frame current_frame;
    struct sockaddr_storage sockaddr;
    unsigned int socklen = sizeof sockaddr;
    evutil_socket_t udp_fd;
    evutil_socket_t tap_fd;
    int err;
    struct endpoint e;

    memset(&e, 0, sizeof(e));
    udp_fd = s->udp->fd;
    tap_fd = s->tap_fd;
    memset(&current_frame, 0, sizeof current_frame);
    while((err = frame_recvfrom(ctx,
                                udp_fd,
                                &current_frame,
                                (struct sockaddr *)&sockaddr,
                                &socklen)) != -1)
    {
        endpoint_init(&e, (struct sockaddr *)&sockaddr, socklen);
        log_debug("[UDP] recving %d(%-#2x) from %s",
                  current_frame.size,
                  current_frame.size,
                  endpoint_presentation(&e));

        /* And forward it to anyone else but except current peer*/
        forward_udp_frame_to_other_peers(ctx,
                                         s->udp,
                                         &current_frame,
                                         endpoint_addr(&e),
                                         endpoint_addrlen(&e));
#if defined Windows
        /*
         * Send to current frame to the windows thread handling the tun/tap
         * devices and clean the evbuffer
         */
        send_buffer_to_device_thread(s, &current_frame);
#else
        /* Write the current frame on the device and clean the evbuffer*/
        async_write(ctx,
                    tap_fd,
                    current_frame.frame,
                    current_frame.size);
#endif
        frame_free(&current_frame);
    }
}

void
udp_peer_free(struct udp_peer const *u)
{
    if (u->_bio_backend != NULL)
        BIO_free(u->_bio_backend);
    if (u->bio != NULL)
        BIO_free(u->bio);
    if (u->ssl != NULL)
        SSL_free(u->ssl);
}

void
server_udp_exit(struct udp *udp)
{
    close(udp->fd);
    SSL_CTX_free(udp->ctx);
    v_udp_foreach(udp->udp_peers, udp_peer_free);
    v_udp_delete(udp->udp_peers);
    sched_fiber_delete(udp->udp_brd_fib);
    sched_fiber_delete(udp->udp_recv_fib);
}

int
server_udp_init(struct server *s,
                struct udp *udp,
                struct endpoint *e)
{
    int             err;
    struct endpoint tmp_endpoint;
    evutil_socket_t tmp_sock = 0;

    tmp_sock = tnt_udp_socket(endpoint_addr(e)->sa_family);
    endpoint_copy(&tmp_endpoint, e);

    if (tmp_sock == -1)
    {
        log_warn("[INIT] [UDP] socket creation failed:");
        return -1;
    }
    endpoint_set_port(&tmp_endpoint, 0); /* Means random port */
    err = bind(tmp_sock,
               endpoint_addr(&tmp_endpoint),
               tmp_endpoint.addrlen);

    if (err == -1)
    {
        log_warn("[INIT] [UDP] binding: ");
        return -1;
    }

    endpoint_assign_sockname(tmp_sock, &tmp_endpoint);
    log_debug("[INIT] [UDP] udp listen on %s",
              endpoint_presentation(&tmp_endpoint));

    err = evutil_make_socket_nonblocking(tmp_sock);
    if (err == -1)
    {
        return -1;
    }
    endpoint_copy(&udp->udp_endpoint, &tmp_endpoint);
    udp->fd = tmp_sock;
    udp->udp_peers = v_udp_new();
    udp->udp_recv_fib = sched_new_fiber(s->ev_sched, server_udp, (intptr_t)s);
    udp->udp_brd_fib = sched_new_fiber(s->ev_sched, broadcast_udp, (intptr_t)s);
    udp->ctx = create_udp_ctx();
    return 0;
}

void
server_udp_launch(struct udp *u)
{
    if (u->udp_brd_fib != NULL)
        sched_fiber_launch(u->udp_brd_fib);
    if (u->udp_recv_fib != NULL)
        sched_fiber_launch(u->udp_recv_fib);
    /*if (u->udp_demux != NULL)
        sched_fiber_launch(u->udp_demux);*/
}

struct udp *
server_udp_new(struct server *s,
               struct endpoint *e)
{
    int err;
    struct udp *udp;
    
    udp = tnt_new(struct udp);
    if (udp == NULL)
    {
        return NULL;
    }
    err = server_udp_init(s, udp, e);
    if (err == -1)
    {
        free(udp);
        return NULL;
    }
    return udp;
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
                         (int *)socklen);
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
    err = async_recv(ctx,
                     fd,
                     (char *)frame->raw_packet,
                     frame->size + sizeof(struct packet_hdr),
                     0);
    return err;
}

unsigned short
udp_get_port(struct udp *udp)
{
    return endpoint_port(&udp->udp_endpoint);
}

