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
    log_debug("Not callback set for %s", __FUNCTION__);
}

void
mc_write_cb(struct bufferevent *bev, void *ctx)
{
    (void)bev;
    (void)ctx;
    log_debug("Not callback set for %s", __FUNCTION__);
}

void
mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    (void)bev;
    (void)events;
    (void)ctx;
    log_debug("Not callback set for %s", __FUNCTION__);
}

void
mc_init(struct mc *this, struct sockaddr *s, int len, struct bufferevent *bev)
{
    struct sockaddr *tmp = malloc(len);

    if (tmp == NULL)
    {
        log_warn("Failed to allocate the memory needed to establish a new "
                 "meta-connection");
        return;
    }
    memcpy(tmp, s, len);
    this->bev = bev;
    this->p.address = tmp;
    this->p.len = len;
    bufferevent_setcb(bev, mc_read_cb, mc_write_cb, mc_event_cb, this);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void
mc_close(struct mc *this)
{
    free(this->p.address);
    bufferevent_free(this->bev);
}
