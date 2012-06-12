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
#if !defined Windows
#include <unistd.h>
#else
#include <io.h>
#include <WS2tcpip.h>
#define write(A, B, C) windows_fix_write(A, B, C)
#define read(A, B, C) windows_fix_read(A, B, C)
#define ssize_t SSIZE_T
#endif

#include "mc.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"

union chartoshort {
    unsigned char *cptr;
    unsigned short *sptr;
}; /* The goal of this union is to properly convert uchar* to ushort* */

#if defined Windows

OVERLAPPED gl_overlap;

static ssize_t
windows_fix_write(intptr_t fd, void *buf, size_t len)
{
	DWORD n = 0;
	int err;

	err = WriteFile((HANDLE)fd, buf, len, &n, &gl_overlap);
	if (err == 0 && GetLastError() != ERROR_IO_PENDING)
	{
		printf("Write failed, error %d\n", GetLastError());
		return -1;
	}
	else
		return n;
}

static ssize_t
windows_fix_read(intptr_t fd, void *buf, size_t len)
{
	DWORD n = 0;
	int err;

	err = ReadFile((HANDLE)fd, buf, len, &n, &gl_overlap);
	if (err == 0 && GetLastError() != ERROR_IO_PENDING)
	{
		printf("Read failed, error %d\n", GetLastError());
		return -1;
	}
	else
		return n;
}
#endif

static void
server_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = (struct server *)ctx;
    ssize_t n;
    struct evbuffer *buf = NULL;
    unsigned short size;
	intptr_t device_fd;

#if defined Windows
	device_fd = s->devide_fd;
#else
	device_fd = event_get_fd(s->device);
#endif
    buf = bufferevent_get_input(bev);
    while (evbuffer_get_length(buf) != 0)
    {
        /*
         * Using chartoshort union to explicitly convert form uchar* to ushort*
         * without warings.
         */
        union chartoshort u;
        u.cptr = evbuffer_pullup(buf, sizeof(size));
        size = ntohs(*u.sptr);
        if (size > evbuffer_get_length(buf))
        {
            log_debug("Receive an incomplete frame of %d(%-#2x) bytes but "
                      "only %d bytes are available.", size, *(u.sptr),
                      evbuffer_get_length(buf));
            break;
        }
        log_debug("Receive a frame of %d(%-#2x) bytes.", size, *(u.sptr));
        evbuffer_drain(buf, sizeof(size));
		n = write(device_fd, evbuffer_pullup(buf, size), size);
        evbuffer_drain(buf, size);
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
    struct server *s = (struct server *)ctx;

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
        log_notice("The socket closed with error. "
                   "Closing the meta-connexion.");
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            mc_close(mc);
            v_mc_erase(&s->pending_peers, mc);
            log_debug("Socket removed from the pending list.");
        }
        else
        {
            mc = v_mc_find_if(&s->peers, &tmp, _server_match_bev);
            if (mc != v_mc_end(&s->peers))
            {
                mc_close(mc);
                v_mc_erase(&s->peers, mc);
                log_debug("Socket removed from the peer list.");
            }
        }
    }
}

static void
listen_callback(struct evconnlistener *evl, evutil_socket_t fd,
                struct sockaddr *sock, int len, void *ctx)
{
    struct server *s = (struct server *)ctx;
    struct event_base *base = evconnlistener_get_base(evl);
    struct bufferevent *bev = bufferevent_socket_new(base, fd,
                                                     BEV_OPT_CLOSE_ON_FREE);
    log_debug("New connection.");
    if (bev != NULL)
    {
        struct mc mctx;

        /*
         * Set a callback to bev using 
         * bufferevent_setcb.
         */
        mc_init(&mctx, sock, (socklen_t)len, bev);
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
}

static void
broadcast_to_peers(struct server *s)
{
    struct frame *fit = v_frame_begin(&s->frames_to_send);
    struct frame *fite = v_frame_end(&s->frames_to_send);
	struct mc *it = NULL;
	struct mc *ite = NULL;

    for (;fit != fite; fit = v_frame_next(fit))
    {
        unsigned short size_networked = htons(fit->size);
        for (it = v_mc_begin(&s->peers),
             ite = v_mc_end(&s->peers);
             it != ite;
             it = v_mc_next(it))
        {
            int err;
            err = bufferevent_write(it->bev, &size_networked, sizeof(size_networked));
            if (err == -1)
            {
                log_notice("Error while crafting the buffer to send to %p.", it);
                break;
            }
            err = bufferevent_write(it->bev, fit->frame, fit->size);
            if (err == -1)
            {
                log_notice("Error while crafting the buffer to send to %p.", it);
                break;
            }
            log_debug("Adding %d(%-#2x) bytes to %p's output buffer.",
                      fit->size, it);
        }
    }
    v_frame_erase_range(&s->frames_to_send, v_frame_begin(&s->frames_to_send), fite);
}

static void
server_device_cb(evutil_socket_t device_fd, short events, void *ctx)
{
    struct server *s = (struct server *)ctx;

#if defined Windows
	device_fd =	s->devide_fd;
	if (events & EV_TIMEOUT)
#else
    if (events & EV_READ)
#endif
    {
        int _n = 0;
        ssize_t n;
        struct frame tmp;

        while ((n = read(device_fd, &tmp.frame, sizeof(tmp.frame))) > 0)
        {
            tmp.size = (unsigned short)n;/* We cannot read more than a ushort*/
            v_frame_push(&s->frames_to_send, &tmp);
            log_debug("Read a new frame of %d bytes.", n);
            _n++;
        }
#if defined Windows
		if (n == 0 || GetLastError() == ERROR_IO_PENDING)
#else
        if (n == 0 || EVUTIL_SOCKET_ERROR() == EAGAIN) /* no errors occurs*/
#endif
        {
            log_debug("Read %d frames in this pass.", _n);
            broadcast_to_peers(s);
        }
        else if (n == -1)
            log_warn("Read on the device failed:");
    }
}

void
server_set_device(struct server *s, int fd)
{
    struct event_base *evbase = evconnlistener_get_base(s->srv);
    struct event *ev = event_new(evbase, -1, 0, server_device_cb, s);
    int err;

	printf("initial device handle %p\n", fd);
	/*Sadly, this don't work on windows :(*/
#if !defined Windows
    err = evutil_make_socket_nonblocking(fd);
    if (err == -1)
        log_debug("Fuck !");
#else
	s->devide_fd = fd;
	s->tv.tv_sec = 1;
	s->tv.tv_usec = 0;
#endif
    if (ev == NULL)
    {
        log_warn("Failed to allocate the event handler for the device:");
        return;
    }
    s->device = ev;
    event_add(s->device, &s->tv);
    log_notice("Event handler for the device sucessfully configured. Starting "
              "the listener...");
    evconnlistener_enable(s->srv);
    log_notice("Listener started.");
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
        log_notice("Syntax error with the peer address given in the conf.");
        return;
    }

    bev = bufferevent_socket_new(evbase, -1, BEV_OPT_CLOSE_ON_FREE);
    if (bev == NULL)
    {
        log_warn("Unable to allocate a socket for connecting to the peer:");
        return;
    }
    err = bufferevent_socket_connect(bev, (struct sockaddr *)&paddr, (int)plen);
    if (err == -1)
    {
        log_warn("Unable to connect to the peer:");
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

    memset(&addr, 0, sizeof(addr));
    memset(&uaddr, 0, sizeof(uaddr));

    v_mc_init(&s->peers);
    v_mc_init(&s->pending_peers);
    v_frame_init(&s->frames_to_send);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MCPORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    evl = evconnlistener_new_bind(evbase, listen_callback,
                                  s, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                  -1, (struct sockaddr const*)&addr,
                                  sizeof(addr));
    if (evl == NULL)
        return -1;

#if defined Windows
	gl_overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, "Device event");
#endif

    uaddr.sin_family = AF_INET;
    uaddr.sin_port = htons(UDPPORT);
    uaddr.sin_addr.s_addr = INADDR_ANY;

    evconnlistener_set_error_cb(evl, accept_error_cb);
    evconnlistener_disable(evl);
    s->srv = evl;
    if (strcmp(conf_peer_address, "") != 0) /*XXX Dirty hack*/
        server_establish_mc_hostname(s, conf_peer_address);
    return 0;
}

