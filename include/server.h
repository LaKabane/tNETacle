#pragma once
#ifndef SERVER_KW2DIKER
#define SERVER_KW2DIKER

#include <event2/event.h>

struct peer {
  struct event *event;
  struct sockaddr *addr;
  socklen_t addrlen;
};

struct server {
  struct event *listen_socket;
  struct event *device;
  size_t num_clients;
  struct peer *peers;
};

int server_init(struct server *, struct event_base *);
void server_set_device(struct server *, struct event *devent);

#endif /* end of include guard: SERVER_KW2DIKER */
