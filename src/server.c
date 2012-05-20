#include <stdio.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#ifndef Windows
#include <errno.h>
#endif /* !windows */

#include "mc.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"

static void
server_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = ctx;
    char buf[4096];
    int n;

    /*Pour le moment c'est nase ce que je fais*/
    n = bufferevent_read(bev, buf, sizeof(buf));
    log_debug("%s", __PRETTY_FUNCTION__);
    if (n != -1)
    {
        n = write(event_get_fd(s->device), buf, n);
        if (n == -1)
            log_warn("Error while writing on the device");
        else
            log_debug("Write %d on the device", n);
    }
}

static void
server_mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    struct server *s = ctx;

    (void)bev;
    (void)s;
    if (events & BEV_EVENT_CONNECTED)
        log_debug("Connected !!");
    if (events & BEV_EVENT_ERROR)
        log_debug("Error ! %p", EVUTIL_SOCKET_ERROR());
}

static void
listen_callback(struct evconnlistener *evl, evutil_socket_t fd,
                struct sockaddr *sock, int len, void *ctx)
{
    struct server *s = ctx;
    struct event_base *base = evconnlistener_get_base(evl);
    struct bufferevent *bev = bufferevent_socket_new(base, fd,
                                                     BEV_OPT_CLOSE_ON_FREE);
    log_debug("New connection !");
    if (bev != NULL)
    {
        struct mc mctx;

        /*
         * Set a callback to bev using 
         * bufferevent_setcb.
         */
        mc_init(&mctx, sock, len, bev);
        bufferevent_setcb(bev, server_mc_read_cb, NULL, server_mc_event_cb, s);
        v_mc_push(&s->peers, &mctx);
        log_debug("v_mc_push");
    }
    else
    {
        log_debug("Failed to allocate bufferevent");
    }
}

static void
accept_error_cb(struct evconnlistener *evl, void *ctx)
{
    (void)evl;
    (void)ctx;
    log_debug("%s", __PRETTY_FUNCTION__);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));
}

static void
broadcast_to_peers(struct server *s, char *buf, int n)
{
    struct mc *it;
    struct mc *ite;
    int err;

    log_debug("%s", __PRETTY_FUNCTION__);
    for (it = v_mc_begin(&s->peers),
         ite = v_mc_end(&s->peers);
         it != ite;
         it = v_mc_next(it))
    {
        log_debug("Broadcasting to %p", it);
        err = bufferevent_write(it->bev, buf, n);
        if (err == -1)
            log_warn("An error occured while broadcast");
    }
}

static void
server_device_cb(evutil_socket_t fd, short events, void *ctx)
{
    struct server *s = ctx;

    if (events & EV_READ)
    {
        int n;
        char buf[1500];

        n = read(fd, buf, sizeof(buf));
        if (n > 0)
        {
            log_debug("Read %d bytes from device", n);
            broadcast_to_peers(s, buf, n);
        }
        else
        {
            log_debug("Error while reading on the device: %s",
                      strerror(errno));
        }
    }
}

void
server_set_device(struct server *s, int fd)
{
    struct event_base *evbase = evconnlistener_get_base(s->srv);
    struct event *ev = event_new(evbase, fd, EV_READ | EV_PERSIST, server_device_cb, s);

    if (ev == NULL)
    {
        log_warn("Failed to allocate the event handler for the device");
        return;
    }
    s->device = ev;
    event_add(s->device, NULL);
    log_debug("Event handler for the device sucessfully configured. Starting "
              "the listener..");
    evconnlistener_enable(s->srv);
    log_debug("Listener started.");
}

extern char conf_peer_address[];

static
void server_establish_mc_hostname(struct server *s, char *hostname)
{
    struct event_base *evbase = evconnlistener_get_base(s->srv);
    struct sockaddr_storage paddr;
    socklen_t plen = sizeof(paddr);
    struct bufferevent *bev;
    struct mc mctx;
    int err;

    err = evutil_parse_sockaddr_port(hostname,
                                     (struct sockaddr*)&paddr,
                                     (int*)&plen);
    if (err == -1)
    {
        log_warn("Syntax error with the peer address given in the conf.");
        return;
    }

    bev = bufferevent_socket_new(evbase, -1, BEV_OPT_CLOSE_ON_FREE);
    if (bev == NULL)
    {
        log_warn("Unable to allocate a socket for connecting to the peer.");
        return;
    }
    err = bufferevent_socket_connect(bev, (struct sockaddr *)&paddr, plen);
    if (err == -1)
    {
        log_warn("Unable to connect to the peer.");
        return;
    }
    mc_init(&mctx, (struct sockaddr*)&paddr, plen, bev);
    bufferevent_setcb(bev, server_mc_read_cb, NULL, server_mc_event_cb, s);
    v_mc_push(&s->peers, &mctx);
    log_debug("v_mc_push");
    log_notice("Connected to %s", hostname);
}

int
server_init(struct server *s, struct event_base *evbase)
{
    struct sockaddr_in addr;
    struct sockaddr_in uaddr;
    struct evconnlistener *evl;
    evutil_socket_t udpsocket;
    int errcode;

    memset(&addr, 0, sizeof(addr));
    memset(&uaddr, 0, sizeof(uaddr));

    v_mc_init(&s->peers);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MCPORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    evl = evconnlistener_new_bind(evbase, listen_callback,
                                  s, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                  -1, (struct sockaddr const*)&addr,
                                  sizeof(addr));
    if (evl == NULL)
        return -1;

    uaddr.sin_family = AF_INET;
    uaddr.sin_port = htons(UDPPORT);
    uaddr.sin_addr.s_addr = INADDR_ANY;

    udpsocket = tnt_udp_socket(IPv4);
    errcode = evutil_make_listen_socket_reuseable(udpsocket);
    if (errcode < 0)
        log_warn("Failed to make the listen UDP socket reusable");

    errcode = evutil_make_socket_nonblocking(udpsocket);
    if (errcode < 0)
        log_warn("Failed to make the listen UDP socket non blocking");

    errcode = bind(udpsocket, (struct sockaddr *)&uaddr, sizeof(uaddr));
    if (errcode < 0)
        log_warn("Failed to bind the listen UDP socket");

    evconnlistener_set_error_cb(evl, accept_error_cb);
    evconnlistener_disable(evl);
    s->srv = evl;
    if (strcmp(conf_peer_address, "") != 0)
        server_establish_mc_hostname(s, conf_peer_address);
    return 0;
}

