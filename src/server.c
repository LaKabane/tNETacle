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

#include <sys/types.h>
#if !defined WIN32
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined Windows
# include <WS2tcpip.h>
# include <io.h>
# define write _write
# define ssize_t SSIZE_T
#endif

#if defined Unix
# include <unistd.h>
# include <netinet/in.h>
#endif

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

#include "tnetacle.h"
#include "options.h"
#include "mc.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"
#include "hexdump.h"

extern struct options serv_opts;

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
	//log_debug("WRITE fd=%ld, buf=%p, len=%ld, n=%ld, err=%d", fd, buf, len, n, err);
	if (err == 0 && GetLastError() != ERROR_IO_PENDING)
	{
		printf("Write failed, error %d\n", GetLastError());
		return -1;
	}
	else
	{
		if (len > 0)
		{
			log_debug("== WRITE == WRITE == WRITE == WRITE ==");
			hex_dump_chk(buf, len);
			log_debug("== WRITE == WRITE == WRITE == WRITE ==");
		}
		WaitForSingleObject(gl_overlap.hEvent, INFINITE);
		return n;
	}
}

static ssize_t
windows_fix_read(intptr_t fd, void *buf, size_t len)
{
	DWORD n = 0;
	int err;

	err = ReadFile((HANDLE)fd, buf, len, &n, &gl_overlap);
	//log_debug("READ fd=%ld, buf=%p, len=%ld, n=%ld, err=%d", fd, buf, len, n, err);
	if (err == 0 && GetLastError() != ERROR_IO_PENDING)
	{
		printf("Read failed, error %d\n", GetLastError());
		return -1;
	}
	else
	{
		if (n > 0)
		{
			log_debug("== READ == READ == READ == READ ==");
			hex_dump_chk(buf, n);
			log_debug("== READ == READ == READ == READ ==");
		}
		WaitForSingleObject(gl_overlap.hEvent, 1000);
		return n;
	}
}
#endif

static void
server_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = (struct server *)ctx;
    ssize_t n;
    struct evbuffer *buf = NULL;
	struct mc *it = NULL;
	struct mc *ite = NULL;
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
            log_debug("receive an incomplete frame of %d(%-#2x) bytes but "
                      "only %d bytes are available", size, *(u.sptr),
                      evbuffer_get_length(buf));
            break;
        }
        log_debug("receive a frame of %d(%-#2x) bytes", size, *(u.sptr));
        for (it = v_mc_begin(&s->peers),
             ite = v_mc_end(&s->peers);
             it != ite;
             it = v_mc_next(it))
        {
            int err;

			if (it->bev == bev)
				continue;
			err = bufferevent_write(it->bev, evbuffer_pullup(buf, sizeof(size) + size), sizeof(size) + size);
            if (err == -1)
            {
                log_notice("error while crafting the buffer to send to %p", it);
                break;
            }
            log_debug("adding %d(%-#2x) bytes to %p's output buffer",
                      size, size, it);
        }
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
        log_debug("moving the state from pending to connected");
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            memcpy(&tmp, mc, sizeof(tmp));
            v_mc_erase(&s->pending_peers, mc);
            v_mc_push(&s->peers, &tmp);
            log_debug("state moved");
        }
    }
    if (events & BEV_EVENT_ERROR)
    {
        struct mc *mc;
        struct mc tmp;

        tmp.bev = bev;
        log_notice("the socket closed with error closing the meta-connexion");
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            mc_close(mc);
            v_mc_erase(&s->pending_peers, mc);
            log_debug("socket removed from the pending list");
        }
        else
        {
            mc = v_mc_find_if(&s->peers, &tmp, _server_match_bev);
            if (mc != v_mc_end(&s->peers))
            {
                mc_close(mc);
                v_mc_erase(&s->peers, mc);
                log_debug("socket removed from the peer list");
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
    log_debug("new connection");
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
        log_debug("failed to allocate bufferevent");
    }
}

static void
accept_error_cb(struct evconnlistener *evl, void *ctx)
{
    (void)evl;
    (void)ctx;
}

/*static */void
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
                log_notice("error while crafting the buffer to send to %p", it);
                break;
            }
            err = bufferevent_write(it->bev, fit->frame, fit->size);
            if (err == -1)
            {
                log_notice("error while crafting the buffer to send to %p", it);
                break;
            }
            log_debug("adding %d(%-#2x) bytes to %p's output buffer",
                      fit->size, fit->size, it);
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
            //log_debug("Read a new frame of %d bytes.", n);
            _n++;
        }
#if defined Windows
		if (n == 0 || GetLastError() == ERROR_IO_PENDING)
#else
        if (n == 0 || EVUTIL_SOCKET_ERROR() == EAGAIN) /* no errors occurs*/
#endif
        {
            //log_debug("Read %d frames in this pass.", _n);
            broadcast_to_peers(s);
        }
        else if (n == -1)
            log_warn("read on the device failed:");
    }
}

#if defined Windows
void
server_set_device(struct server *s, int fd)
{
    struct event_base *evbase = evconnlistener_get_base(s->srv);
	struct event *ev = event_new(evbase, -1, EV_PERSIST, server_device_cb, s);
    int err;

    /*Sadly, this don't work on windows :(*/
    s->devide_fd = fd;
    s->tv.tv_sec = 0;
    s->tv.tv_usec = 50000;
    if (ev == NULL)
    {
        log_warn("failed to allocate the event handler for the device:");
        return;
    }
    s->device = ev;
    //event_add(s->device, &s->tv);
    log_notice("event handler for the device sucessfully configured");
    evconnlistener_enable(s->srv);
    log_notice("listener started");
}
#else

void
server_set_device(struct server *s, int fd)
{
    struct event_base *evbase = evconnlistener_get_base(s->srv);
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
    log_notice("event handler for the device sucessfully configured");
    evconnlistener_enable(s->srv);
    log_notice("listener started");
}
#endif

int
server_init(struct server *s, struct event_base *evbase)
{
    struct evconnlistener *evl;
    struct bufferevent *bev;
    struct mc mctx;
    struct sockaddr *listens;
    struct sockaddr *peers;
    int err;
    size_t i;

#if defined Windows
	gl_overlap.hEvent = CreateEvent(NULL, /*Auto reset*/FALSE,
		                                  /*Initial state*/FALSE,
										  TEXT("Device event"));
#endif

    peers = v_sockaddr_begin(&serv_opts.peer_addrs);

    v_mc_init(&s->peers);
    v_mc_init(&s->pending_peers);
    v_frame_init(&s->frames_to_send);

    /*evbase = evconnlistener_get_base(s->srv);*/

    /* Listen on all ListenAddress */
    for (listens = v_sockaddr_begin(&serv_opts.listen_addrs);
         listens != NULL && listens != v_sockaddr_end(&serv_opts.listen_addrs);
         listens = v_sockaddr_next(listens)) {
        evl = evconnlistener_new_bind(evbase, listen_callback,
          s, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
          listens, sizeof(*listens));
        if (evl == NULL) {
            log_debug("fail at ListenAddress #%i", i);
            return -1;
        }
    }
    evconnlistener_set_error_cb(evl, accept_error_cb);
    evconnlistener_disable(evl);
    s->srv = evl;

    /* If we don't have any PeerAddress it's finished */
    if (serv_opts.peer_addrs.size == 0)
        return 0;

    for (;peers != NULL && peers != v_sockaddr_end(&serv_opts.peer_addrs);
         peers = v_sockaddr_next(peers)) {
        bev = bufferevent_socket_new(evbase, -1, BEV_OPT_CLOSE_ON_FREE);
        if (bev == NULL) {
            log_warn("unable to allocate a socket for connecting to the peer");
	    break;
        }
        err = bufferevent_socket_connect(bev, peers, sizeof(*peers));
        if (err == -1) {
            log_warn("unable to connect to the peer");
	    break;
        }
        mc_init(&mctx, peers, sizeof(*peers), bev);
    }

    bufferevent_setcb(bev, server_mc_read_cb, NULL, server_mc_event_cb, s);
    v_mc_push(&s->pending_peers, &mctx);
    return 0;
}
