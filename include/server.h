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

#include <openssl/ssl.h> /* Can not forward declare SSL* types*/
#include "mc.h"

struct evconnlistener;
struct sockaddr;
struct event_base;

#if defined Windows

# define write(A, B, C) windows_fix_write(A, B, C)
# define read(A, B, C) windows_fix_read(A, B, C)
# define ssize_t SSIZE_T

extern OVERLAPPED gl_overlap;
ssize_t windows_fix_write(intptr_t fd, void *buf, size_t len);
ssize_t windows_fix_read(intptr_t fd, void *buf, size_t len);

#endif

#define VECTOR_TYPE struct mc
#define VECTOR_PREFIX mc
#include "vector.h"

struct frame {
  unsigned short size;
  char frame[1542]; /*Max size of a frame with a 1500 MTU (payload + header size)*/
};

#define VECTOR_TYPE struct frame
#define VECTOR_PREFIX frame
#include "vector.h"

#define VECTOR_TYPE struct evconnlistener*
#define VECTOR_PREFIX evl
#define VECTOR_TYPE_SCALAR
#include "vector.h"

struct server {
  struct vector_evl srv_list; /*list of the listenners*/
  struct event *udp_endpoint;
  struct event *device;
#if defined Windows
  intptr_t devide_fd;
  struct timeval tv;
#endif
  struct vector_mc peers; /* The actual list of peers */
  struct vector_mc pending_peers; /* Pending in connection peers*/
  struct vector_frame frames_to_send;
  SSL_CTX *server_ctx;
  struct event_base *evbase;
};

SSL_CTX *evssl_init(void);
int server_init(struct server *, struct event_base *);
void server_set_device(struct server *, int fd);

#endif /* end of include guard: SERVER_KW2DIKER */
