#include "tnetacle.h"
#include "log.h"
#include "upnp.h"

static void gupnp_prepare_events(struct upnp *);
static void gupnp_process_events(struct upnp *);

static void
gupnp_set_event_callback(evutil_socket_t fd, short events, void *user)
{
  struct upnp *upnp = user;
  int i;
  GPollFD *pfd;

  for (i = 0; i < upnp->req_pfds; ++i)
  {
    pfd = upnp->pfds + i;
    if (pfd->fd == fd)
      break;
  }

  pfd->revents = 0;
  if ((events & EV_READ) && (pfd->events & G_IO_IN))
    pfd->revents |= G_IO_IN;
  if ((events & EV_WRITE) && (pfd->events & G_IO_OUT))
    pfd->revents |= G_IO_OUT;

  log_debug("set_event_callback pfd %i %hu %hu", pfd->fd, pfd->events, pfd->revents);
  gupnp_process_events(upnp);
  gupnp_prepare_events(upnp);
}

static void
gupnp_timeout_callback(evutil_socket_t fd, short events, void *user)
{
  struct upnp *upnp = user;

  log_debug("timeout_callback");
  gupnp_process_events(upnp);
  gupnp_prepare_events(upnp);
}

static void
gupnp_prepare_events(struct upnp *upnp)
{
  int i, reqtimeout;
  int n_fds = GUPNP_FDS_SIZE;

  log_debug("gupnp_prepare_events");
  if (!g_main_context_acquire(upnp->gctx)) {
    log_warn("fuck you I wanna be the owner");
    return ;
  }

  g_main_context_prepare(upnp->gctx, &upnp->priority);
  upnp->req_pfds = g_main_context_query(upnp->gctx, upnp->priority,
      &reqtimeout, upnp->pfds, n_fds);
  if (upnp->req_pfds > n_fds)
  {
    log_warn("too many events to handle (%i for %i slots)",
        upnp->req_pfds + GUPNP_FDS_SIZE, GUPNP_FDS_SIZE);
    upnp->req_pfds = GUPNP_FDS_SIZE;
  }
  else if (upnp->req_pfds < 0)
    goto release;
  log_debug("req_pfds %i", upnp->req_pfds);

  for (i = 0; i < upnp->req_pfds; ++i)
  {
    struct event *ev = upnp->events + i;
    GPollFD *pfd = upnp->pfds + i;
    short mask = 0;

    if (pfd->events & G_IO_IN)
      mask |= EV_READ;
    if (pfd->events & G_IO_OUT)
      mask |= EV_WRITE;
    if (mask)
    {
      log_debug("add event pfd %i", pfd->fd);
      event_assign(ev, upnp->evbase, pfd->fd, mask | EV_PERSIST, gupnp_set_event_callback, upnp);
      event_add(ev, NULL);
    }
  }
  if (reqtimeout > 0)
  {
    struct timeval timeout;
    
    log_debug("reqtimeout %i", reqtimeout);
    timeout.tv_sec = reqtimeout / 1000;
    timeout.tv_usec = (reqtimeout % 1000) * 1000;
    evtimer_add(&upnp->timeout, &timeout);
    log_debug("add timeout %i", reqtimeout);
  }
release:
  g_main_context_release(upnp->gctx);
  log_debug("end gupnp_prepare_events");
}

static void
gupnp_process_events(struct upnp *upnp)
{
  int i;

  log_debug("gupnp_process_events");
  if (!g_main_context_acquire(upnp->gctx)) {
    log_warn("fuck you I wanna be the owner");
    return ;
  }

  if (g_main_context_check(upnp->gctx, upnp->priority, upnp->pfds, upnp->req_pfds))
    g_main_context_dispatch(upnp->gctx);

  for (i = 0; i < upnp->req_pfds; ++i)
  {
    struct event *ev = upnp->events + i;

    log_debug("del event pfd %i", event_get_fd(ev));
    event_del(ev);
  }
  evtimer_del(&upnp->timeout);
release:
  g_main_context_release(upnp->gctx);
  log_debug("end gupnp_process_events");
}

int
tnt_upnp_async_init(struct upnp *upnp, struct event_base *evbase)
{
  g_type_init();

  upnp->gctx = g_main_context_default();
  upnp->evbase = evbase;
  upnp->priority = -1;
  evtimer_assign(&upnp->timeout, evbase, gupnp_timeout_callback, upnp);
  upnp->req_pfds = G_PRIORITY_DEFAULT;

  upnp->igd_async = gupnp_simple_igd_new();
  gupnp_process_events(upnp);
  gupnp_prepare_events(upnp);
  return 0;
}

int
tnt_upnp_async_bind_mapped_port(struct upnp *upnp,
                                upnp_mapped_port_callback func)
{
  g_signal_connect(upnp->igd_async, "mapped-external-port",
      G_CALLBACK(func), NULL);
  gupnp_process_events(upnp);
  gupnp_prepare_events(upnp);
  return 0;
}

int
tnt_upnp_async_bind_error_mapping(struct upnp *upnp,
                                  upnp_error_mapping_callback func)
{
  g_signal_connect(upnp->igd_async, "error-mapping-port",
      G_CALLBACK(func), NULL);
  gupnp_process_events(upnp);
  gupnp_prepare_events(upnp);
  return 0;
}

int
tnt_upnp_async_add_port(struct upnp *upnp, char *proto, short iport,
                        char *addr, short dport)
{
  gupnp_simple_igd_add_port(upnp->igd_async, proto, iport, addr, dport, 3600,
      "tNETacle");
  gupnp_process_events(upnp);
  gupnp_prepare_events(upnp);
  return 0;
}
