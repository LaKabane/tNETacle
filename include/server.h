/**
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**/

#pragma once
#ifndef SERVER_KW2DIKER
#define SERVER_KW2DIKER

#include "networking.h"

/*
 * We include this one after networking.h because openssl includes windows.h
 * leading to a redifinition of most of the wsaapi symbols on Windows.
 * Seriously, fuck you OpenSSL.
 */
#include <openssl/ssl.h> /* Can not forward declare SSL* types*/

#include "udp.h"

struct evconnlistener;
struct bufferevent;
struct event_base;
struct vector_evl;
struct vector_mc;
struct sockaddr;
struct frame;
struct mc;

#define VECTOR_TYPE struct mc
#define VECTOR_PREFIX mc
#define VECTOR_FORWARD
#include "vector.h"

#define VECTOR_TYPE struct evconnlistener*
#define VECTOR_PREFIX evl
#define VECTOR_TYPE_SCALAR
#define VECTOR_FORWARD
#include "vector.h"

#define VECTOR_TYPE struct frame
#define VECTOR_PREFIX frame
#define VECTOR_FORWARD
#include "vector.h"


#if defined Windows
# define ssize_t SSIZE_T
#endif

#pragma pack(push, 1)
struct packet_hdr
{
    unsigned short size;
};
#pragma pack(pop)

struct server 
{
  struct vector_evl     *srv_list; /*list of the listenners*/
  struct udp            udp;
  struct event          *device;
  struct vector_mc      *peers; /* The actual list of peers */
  struct vector_mc      *pending_peers; /* Pending in connection peers*/
  struct vector_frame   *frames_to_send;
  struct event_base     *evbase;
  SSL_CTX               *server_ctx;
  struct mc             mc_client;
#if defined Windows
  struct bufferevent    *pipe_endpoint;
#endif
};

SSL_CTX *evssl_init(void);

int server_init(struct server *,
                struct event_base *);

void server_set_device(struct server *,
                       int fd);

#if defined Windows
void broadcast_udp_to_peers(struct server *s);

int frame_alloc(struct frame *,
                unsigned int size);
#endif

/* I didn't want to do this */
void server_mc_event_cb(struct bufferevent *bev,
                        short events,
                        void *ctx);

void server_mc_read_cb(struct bufferevent *bev,
                       void *ctx);

#endif /* end of include guard: SERVER_KW2DIKER */
