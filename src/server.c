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
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "mc.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"

static void
server_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = ctx;
    char buf[4096];
    ssize_t n;

    /*Pour le moment c'est nase ce que je fais*/
    n = bufferevent_read(bev, buf, sizeof(buf));
    if (n != -1)
    {
        n = write(event_get_fd(s->device), buf, n);
        if (n == -1)
            log_warn("Error while writing on the device");
        else
            log_debug("Write %d on the device", n);
    }
}

static int
_server_match_bev(struct mc const *a, struct mc const *b)
{
    return a->bev == b->bev;
}

static void
server_mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    struct server *s = ctx;

    (void)bev;
    (void)s;
    if (events & BEV_EVENT_CONNECTED)
    {
        struct mc *mc;
        struct mc tmp;

        tmp.bev = bev;
        log_debug("Moving the state form pending to connected.");
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            memcpy(&tmp, mc, sizeof(tmp));
            v_mc_erase(&s->pending_peers, mc);
            v_mc_push(&s->peers, &tmp);
            log_debug("State moved");
        }
    }
    if (events & BEV_EVENT_ERROR)
    {
        struct mc *mc;
        struct mc tmp;

        tmp.bev = bev;
        log_debug("The socket closed with error. "
                  "Closing the meta-connexion.");
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            mc_close(mc);
            v_mc_erase(&s->pending_peers, mc);
            log_debug("Socket removed");
        }
        else
        {
            mc = v_mc_find_if(&s->peers, &tmp, _server_match_bev);
            if (mc != v_mc_end(&s->peers))
            {
                mc_close(mc);
                v_mc_erase(&s->peers, mc);
                log_debug("Socket removed");
            }
        }
    }
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
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));
}

static void
broadcast_to_peers(struct server *s, char *buf, size_t n)
{
    struct mc *it;
    struct mc *ite;
    int err;

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
        ssize_t n;
        char buf[1500];

        n = read(fd, buf, sizeof(buf));
        if (n > 0)
        {
            log_debug("Read %d bytes from device", n);
            broadcast_to_peers(s, buf, (size_t)n);
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
    v_mc_push(&s->pending_peers, &mctx);
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
    v_mc_init(&s->pending_peers);
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

