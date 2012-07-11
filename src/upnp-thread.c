#include "upnp.h"

int
tnt_upnp_thread_init(struct upnp *upnp, struct event_base *evbase)
{
  g_type_init();
  upnp->igd_thread = gupnp_simple_igd_thread_new();
  return 0;
}


int
tnt_upnp_thread_add_port(struct upnp *upnp, char *proto, short iport,
                        char *addr, short dport)
{
  gupnp_simple_igd_add_port(GUPNP_SIMPLE_IGD(upnp->igd_thread), proto, iport,
      addr, dport, 3600, "tNETacle");
  return 0;
}

int
tnt_upnp_thread_bind_mapped_port(struct upnp *upnp,
                                upnp_mapped_port_callback func)
{
  g_signal_connect(upnp->igd_thread, "mapped-external-port",
      G_CALLBACK(func), NULL);
  return 0;
}

int
tnt_upnp_thread_bind_error_mapping(struct upnp *upnp,
                                  upnp_error_mapping_callback func)
{
  g_signal_connect(upnp->igd_thread, "error-mapping-port",
      G_CALLBACK(func), NULL);
  return 0;
}
