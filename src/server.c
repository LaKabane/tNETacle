#include <stdio.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

#ifndef Windows
#include <errno.h>
#endif /* !windows */

#include "mc-endpoint.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"

static void
listen_callback(struct evconnlistener *evl, evutil_socket_t fd,
		struct sockaddr *sock, int len, void *ctx)
{
    struct server *s = ctx;
    struct event_base *base = evconnlistener_get_base(evl);
    struct bufferevent *bev = bufferevent_socket_new(base, fd,
						     BEV_OPT_CLOSE_ON_FREE);
    if (bev != NULL)
    {
	struct mc mctx;

	log_debug("%s", __PRETTY_FUNCTION__);
	mc_init(&mctx, sock, len, bev);
	v_mc_push(&s->peers, &mctx);
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

void
server_set_device(struct server *s, struct event *devent)
{
  s->device = devent;
  log_debug("%s", __PRETTY_FUNCTION__);
  evconnlistener_enable(s->srv);
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
  return 0;
}

