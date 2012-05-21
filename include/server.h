#pragma once
#ifndef SERVER_KW2DIKER
#define SERVER_KW2DIKER

#include "mc.h"

struct evconnlistener;
struct sockaddr;
struct event_base;

#define VECTOR_TYPE struct mc
#define VECTOR_PREFIX mc
#include "vector.h"
#undef VECTOR_PREFIX
#undef VECTOR_TYPE

struct server {
  struct evconnlistener *srv;
  struct event *udp_endpoint;
  struct event *device;
  struct vector_mc peers; /* The actual list of peers */
  struct vector_mc pending_peers; /* list of peers waiting for the connexion to finish */
};

#define UDPPORT 8989
#define MCPORT 4242

int server_init(struct server *, struct event_base *);
void server_set_device(struct server *, int fd);

#endif /* end of include guard: SERVER_KW2DIKER */
