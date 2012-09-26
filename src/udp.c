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

#define VECTOR_TYPE struct udp_peer
#define VECTOR_PREFIX udp
#define VECTOR_NON_STATIC
#include "vector.h"

void
forward_udp_frame_to_other_peers(struct server *s,
                                 struct frame *current_frame,
                                 struct sockaddr *current_sockaddr,
                                 unsigned int
                                 current_socklen) 
{
    struct mc *it = NULL;
    struct mc *ite = NULL;
    char name[INET6_ADDRSTRLEN];
    struct sockaddr_storage udp_storage;

    (void)current_socklen;
    for (it = v_mc_begin(s->peers), ite = v_mc_end(s->peers);
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
        err = sendto(s->udp.fd,
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

void
broadcast_udp_to_peers(struct server *s)
{
    struct frame *fit = v_frame_begin(s->frames_to_send);
    struct frame *fite = v_frame_end(s->frames_to_send);
    struct sockaddr_storage udp_addr;
    char name[512];

    /* For all the frames*/
    for (;fit != fite; fit = v_frame_next(fit))
    {
        struct mc *it = NULL;
        struct mc *ite = NULL;

        it = v_mc_begin(s->peers);
        ite = v_mc_end(s->peers);
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
            err = sendto(s->udp.fd,
                         fit->raw_packet,
                         fit->size + sizeof(struct packet_hdr), 0,
                         (struct sockaddr const *)&udp_addr,
                         it->p.len);
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


int
server_init_udp(struct server *s,
                struct sockaddr *addr,
                int len)
{
    int err;
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

        sin->sin_port = htons(TNETACLE_UDP_PORT);
    }
    else if (udp_addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&udp_addr;

        sin6->sin6_port = htons(TNETACLE_UDP_PORT);
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
    udp->fd = tmp_sock;
    udp->udp_sched = sched_new(evbase);
    udp->udp_fiber = sched_new_fiber(udp->udp_sched, server_udp, s);
    sched_launch(udp->udp_sched);
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

    /* lol */
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

