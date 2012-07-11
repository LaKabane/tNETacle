#ifndef UPNP_H
#define UPNP_H

#include "event2/event.h"
#include "event2/event_struct.h"
#if defined USE_GUPNP
#include "glib.h"
#include "libgupnp-igd/gupnp-simple-igd-thread.h"
//#include "libgupnp-igd/gupnp-simple-igd.h"
#endif

struct upnp
{
  int enabled;
#if defined USE_GUPNP
#define GUPNP_FDS_SIZE  128
  GUPnPSimpleIgdThread *igd_thread;
  GUPnPSimpleIgd *igd_async;
  GMainContext *gctx;
  struct event_base *evbase;
  int priority;
  int req_pfds;
  GPollFD pfds[GUPNP_FDS_SIZE];
  struct event events[GUPNP_FDS_SIZE];
  struct event timeout;
#endif
};

#if defined USE_GUPNP
typedef void (*upnp_mapped_port_callback)(GUPnPSimpleIgd *igd, gchar *proto,
    gchar *external_ip, gchar *replaces_external_ip, guint external_port,
    gchar *local_ip, guint local_port, gchar *description, gpointer user);
typedef void (*upnp_error_mapping_callback)(GUPnPSimpleIgd *igd, GError *error, gchar *proto,
    guint external_port, gchar *description, gpointer user);
#endif

int tnt_upnp_init(struct upnp *, struct event_base *);
int tnt_upnp_add_port(struct upnp *);

int tnt_upnp_async_init(struct upnp *, struct event_base *);
int tnt_upnp_async_bind_mapped_port(struct upnp *, upnp_mapped_port_callback);
int tnt_upnp_async_bind_error_mapping(struct upnp *, upnp_error_mapping_callback);
int tnt_upnp_async_add_port(struct upnp *, char *, short, char *, short);

int tnt_upnp_thread_init(struct upnp *, struct event_base *);
int tnt_upnp_thread_bind_mapped_port(struct upnp *, upnp_mapped_port_callback);
int tnt_upnp_thread_bind_error_mapping(struct upnp *, upnp_error_mapping_callback);
int tnt_upnp_thread_add_port(struct upnp *, char *, short, char *, short);

#endif /* UPNP_H */
