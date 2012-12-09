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

#ifndef UDP_US4EZ32H
#define UDP_US4EZ32H

#include <openssl/sha.h>
#include <dtls.h>
#include <event2/util.h>
#include "networking.h"
#include "coro.h"
#include "tntsched.h"
#include "endpoint.h"

#define TNETACLE_UDP_PORT   7676
#define UDP_MTU             1500

struct server;
struct frame;
struct sockaddr;
struct event;

struct udp
{
    evutil_socket_t         fd;
    struct dtls_context_t   *ctx;
    struct fiber            *fib;
    struct endpoint         udp_endpoint;
};

struct udp *udp_new(struct server *s);

int udp_init(struct server *s,
                    struct udp *u);

int udp_bind(struct udp *u, struct endpoint *e);

void udp_launch(struct udp *u);

void udp_exit(struct udp *);

int frame_recvfrom(void *ctx,
                   evutil_socket_t fd,
                   struct frame *frame,
                   struct sockaddr *saddr,
                   socklen_t *socklen);

unsigned short
udp_get_port(struct udp *);

int udp_broadcast(struct udp *, char const *data, size_t len);
int udp_connect(struct udp *, struct endpoint *e);

#endif /* end of include guard: UDP_US4EZ32H */
