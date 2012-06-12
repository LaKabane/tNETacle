/**
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**/

#include <stdlib.h>
#include <string.h>
#include <event2/bufferevent.h>
#include <event2/event.h>

#include "mc.h"
#include "log.h"

void mc_read_cb(struct bufferevent *, void *);
void mc_write_cb(struct bufferevent *, void *);
void mc_event_cb(struct bufferevent *, short, void *);

void
mc_read_cb(struct bufferevent *bev, void *ctx)
{
    (void)bev;
    (void)ctx;
    log_debug("No callback set for read");
}

void
mc_write_cb(struct bufferevent *bev, void *ctx)
{
    (void)bev;
    (void)ctx;
    log_debug("No callback set for write");
}

void
mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    (void)bev;
    (void)events;
    (void)ctx;
    log_debug("No callback set for special events");
}

void
mc_init(struct mc *self, struct sockaddr *s, socklen_t len, struct bufferevent *bev)
{
    struct sockaddr *tmp = malloc(len);

    if (tmp == NULL)
    {
        log_notice("Failed to allocate the memory needed to establish a new "
                   "meta-connection");
        return;
    }
    memcpy(tmp, s, len);
    self->bev = bev;
    self->p.address = tmp;
    self->p.len = len;
    bufferevent_setcb(bev, mc_read_cb, mc_write_cb, mc_event_cb, self);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void
mc_close(struct mc *self)
{
    free(self->p.address);
    bufferevent_free(self->bev);
}
