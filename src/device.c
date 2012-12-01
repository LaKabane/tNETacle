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
#include "udp.h"

#include "tnetacle.h"
#include "options.h"
#include "mc.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"
#include "hexdump.h"
#include "wincompat.h"
#include "device.h"
#include "frame.h"
#include "endpoint.h"

#if defined Windows

void
send_buffer_to_device_thread(struct server *s,
                             struct frame *frame)
{
    struct evbuffer *output = bufferevent_get_output(s->pipe_endpoint);

    /*No need to networkize the size, we are in local !*/
    evbuffer_add(output, &frame->size, sizeof frame->size);
    evbuffer_add(output, frame->frame, frame->size);
}

void
server_set_device(struct server *s,
                  intptr_t fd)
{
    struct evconnlistener **it = NULL;
    struct evconnlistener **ite = NULL;

    it = v_evl_begin(s->srv_list);
    ite = v_evl_end(s->srv_list);
    /* Enable all the listeners */
    for (; it != ite; it = v_evl_next(it))
    {
        struct endpoint *listen_endp = endpoint_new();
        evutil_socket_t fd;

        fd = evconnlistener_get_fd(*it);
        endpoint_assign_sockname(fd, listen_endp);
        log_info("[INIT] [TCP] starting server on %s",
            endpoint_presentation(listen_endp));
        evconnlistener_enable(*it);
        free(listen_endp);
    }
    server_udp_launch(s->udp);
    log_info("listeners started");
}

#else

void
server_set_device(struct server *s,
                  intptr_t fd)
{
    struct evconnlistener **it = NULL;
    struct evconnlistener **ite = NULL;
    int err;

    err = evutil_make_socket_nonblocking(fd);
    if (err == -1)
        log_debug("fuck !");

    log_info("event handler for the device sucessfully configured");

    it = v_evl_begin(s->srv_list);
    ite = v_evl_end(s->srv_list);
    /* Enable all the listeners */
    for (; it != ite; it = v_evl_next(it))
    {
        struct endpoint *listen_endp = endpoint_new();
        evutil_socket_t fd;

        fd = evconnlistener_get_fd(*it);
        endpoint_assign_sockname(fd, listen_endp);
        log_info("[INIT] [TCP] starting server on %s",
            endpoint_presentation(listen_endp));
        evconnlistener_enable(*it);
        free(listen_endp);
    }
    s->tap_fd = fd;
    s->device_fib = sched_new_fiber(s->ev_sched, server_device, (intptr_t)s);
    server_udp_launch(s->udp);
    sched_fiber_launch(s->device_fib);
    log_info("listener started");
}

void
server_device(void *async_ctx)
{
    struct server *s = (struct server *)sched_get_userptr(async_ctx);
    evutil_socket_t tap_fd = s->tap_fd;
    ssize_t n;
    struct frame tmp;

    /* I know it sucks. I'm waiting for libtuntap to handle*/
    /* a FIONREAD-like api.*/

    do
    {
        /* 
         * Alloc the memory for a new frame
         *
         * Now, we pre-alloc FRAME_DYN_SIZE, waiting for a portable way to do
         * a sort of fioread
         */

        frame_alloc(&tmp, FRAME_DYN_SIZE);
        async_event(async_ctx, tap_fd, EV_READ);
        while ((n = read(tap_fd, tmp.frame, FRAME_DYN_SIZE)) != -1)
        {
            if (n == -1)
            {
                log_warn("[TAP] read on the device failed");
                break;
            }
            /* Can we read more than a ushort ? */
            tmp.size = (unsigned short)n;
            v_frame_push(s->frames_to_send, &tmp);
            frame_alloc(&tmp, FRAME_DYN_SIZE);
        }

        if (v_frame_size(s->frames_to_send) > 0)
        {
            broadcast_udp_to_peers(s);
        }

        /* Don't forget to free the last allocated frame */
        /* As we are out of the loop, the last call to frame alloc is useless */
        frame_free(&tmp);

    } while(1);

    sched_fiber_exit(async_ctx, -1);
}

#endif

