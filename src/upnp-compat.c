#include "tnetacle.h"
#include "options.h"
#include "log.h"
#include "upnp.h"

extern struct options serv_opts;

#if defined USE_GUPNP
static void
gupnp_mapped_external_port(GUPnPSimpleIgd *igd, gchar *proto,
    gchar *external_ip, gchar *replaces_external_ip, guint external_port,
    gchar *local_ip, guint local_port, gchar *description, gpointer user)
{
  log_info("proto:%s ex:%s oldex:%s exp:%u local:%s localp:%u desc:%s",
           proto, external_ip, replaces_external_ip, external_port, local_ip,
           local_port, description);
}

static void
gupnp_error_mapping_port(GUPnPSimpleIgd *igd, GError *error, gchar *proto,
    guint external_port, gchar *description, gpointer user)
{
  log_warn("proto:%s port:%u desc:%s error: %s:%d %s", proto, external_port,
           description, g_quark_to_string(error->domain),
           error->code, error->message);
}
#endif

int
tnt_upnp_init(struct upnp *upnp, struct event_base *evbase)
{
#if defined USE_GUPNP
  tnt_upnp_thread_init(upnp, evbase);
  tnt_upnp_thread_bind_mapped_port(upnp, gupnp_mapped_external_port);
  tnt_upnp_thread_bind_error_mapping(upnp, gupnp_error_mapping_port);
#endif
}

int
tnt_upnp_add_port(struct upnp *upnp)
{
  int i;
  struct sockaddr *listens;

  for (listens = v_sockaddr_begin(&serv_opts.listen_addrs);
       listens != NULL && listens != v_sockaddr_end(&serv_opts.listen_addrs);
       listens = v_sockaddr_next(listens)) {
    short port = ntohs(((struct sockaddr_in *)listens)->sin_port);
#if defined USE_GUPNP
    tnt_upnp_thread_add_port(upnp, "TCP", port, 
        inet_ntoa(((struct sockaddr_in *)listens)->sin_addr), port);
#endif
  }
}
