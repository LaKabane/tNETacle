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
                  int fd)
{
    struct evconnlistener **it = NULL;
    struct evconnlistener **ite = NULL;

    it = v_evl_begin(s->srv_list);
    ite = v_evl_end(s->srv_list);
    /* Enable all the listeners */
    for (; it != ite; it = v_evl_next(it))
    {
        evconnlistener_enable(*it);
    }
    log_info("listeners started");
}

#else

void
server_set_device(struct server *s,
                  int fd)
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

    it = v_evl_begin(s->srv_list);
    ite = v_evl_end(s->srv_list);
    /* Enable all the listeners */
    for (; it != ite; it = v_evl_next(it))
    {
        evconnlistener_enable(*it);
    }
    log_info("listener started");
}

void
server_device_cb(evutil_socket_t device_fd,
                 short events,
                 void *ctx)
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
            v_frame_push(s->frames_to_send, &tmp);
            frame_alloc(&tmp, FRAME_DYN_SIZE);
        }
#undef FRAME_DYN_SIZE
        if (n == 0 || EVUTIL_SOCKET_ERROR() == EAGAIN) /* no errors occurs*/
        {
            broadcast_udp_to_peers(s);
        }
        else if (n == -1)
            log_warn("read on the device failed:");
        /* Don't forget to free the last allocated frame */
        /* As we are out of the loop, the last call to frame alloc is useless */
        frame_free(&tmp);
    }
}

#endif

