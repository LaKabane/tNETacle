#include "mc.h"
#include "log.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

void mc_read_cb(struct bufferevent *, void *);
void mc_write_cb(struct bufferevent *, void *);
void mc_event_cb(struct bufferevent *, short, void *);

void
mc_read_cb(struct bufferevent *bev, void *ctx)
{
    (void)bev;
    (void)ctx;
    log_debug("%s\n", __PRETTY_FUNCTION__);
}

void
mc_write_cb(struct bufferevent *bev, void *ctx)
{
    (void)bev;
    (void)ctx;
    log_debug("%s\n", __PRETTY_FUNCTION__);
}

void
mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    (void)bev;
    (void)events;
    (void)ctx;
    log_debug("%s\n", __PRETTY_FUNCTION__);
}

void
mc_init(struct mc *this, struct sockaddr *s, int len, struct bufferevent *bev)
{
    this->bev = bev;
    this->p.address = s;
    this->p.len = len;
    bufferevent_setcb(bev, mc_read_cb, mc_write_cb, mc_event_cb, this);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}
