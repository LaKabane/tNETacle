/*
 * Copyright (c) 2012 Florent Tribouilloy <tribou_f AT epitech DOT net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined Unix
# include <unistd.h>
#endif

#include "client.h"

void
client_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = (struct server *)ctx;
    ssize_t n;
    struct evbuffer *buf = NULL;
    //struct frame current_frame;
    unsigned short size;
    unsigned short *network_size_ptr;

    //memset(&current_frame, 0, sizeof current_frame);
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
        //current_frame.size = size;
        //current_frame.frame = evbuffer_pullup(buf, size); 
        /* And forward it to anyone else but except current peer*/
        //forward_frame_to_other_peers(s, &current_frame, bev);

#if defined Windows
        /*
         * Send to current frame to the windows thread handling the tun/tap
         * devices and clean the evbuffer
         */
        send_buffer_to_device_thread(s, &current_frame);
#else
        /* Write the current frame on the device and clean the evbuffer*/
        //n = write(event_get_fd(s->device), current_frame.frame, current_frame.size);
#endif
        evbuffer_drain(buf, size);
    }
}
