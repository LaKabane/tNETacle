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

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include "coro.h"
#include "tntsched.h"

#define TNETACLE_UDP_PORT   7676
#define UDP_MTU             1500

enum udp_ssl_flags
{
    DTLS_ENABLE = (1 << 0),
    DTLS_DISABLE = (1 << 1),
    DTLS_CLIENT = (1 << 2),
    DTLS_SERVER = (1 << 3),
};

struct server;
struct frame;
struct sockaddr;
struct event;
struct vector_frame;

struct udp_peer
{
    struct sockaddr_storage addr;
    int                     socklen;
    BIO                     *bio;
    BIO                     *_bio_backend;
    SSL                     *ssl;
    enum udp_ssl_flags      ssl_flags;
};

#define VECTOR_TYPE struct udp_peer
#define VECTOR_PREFIX udp
#define VECTOR_FORWARD
#include "vector.h"

struct udp
{
    int                     fd;
    int                     udp_addrlen;
    SSL_CTX                 *ctx;
    struct sched            *udp_sched;
    struct fiber            *udp_fiber;
    struct vector_frame     *frame_udp;
    struct vector_udp       *udp_peers;
    struct sockaddr_storage udp_addr;
};

int server_init_udp(struct server *s,
                    struct sockaddr *addr,
                    int len);

void udp_register_new_peer(struct udp *s,
                           struct sockaddr *sock,
                           int socklen,
                           int ssl_flags);

void forward_udp_frame_to_other_peers(struct udp *s,
                                      struct frame *current_frame,
                                      struct sockaddr *current_sockaddr,
                                      unsigned int current_socklen);

void broadcast_udp_to_peers(struct server *s);

int frame_recvfrom(void *ctx,
                   int fd,
                   struct frame *frame,
                   struct sockaddr *saddr,
                   unsigned int *socklen);

void
server_udp(void *ctx);

unsigned short
udp_get_port(struct udp *);

#endif /* end of include guard: UDP_US4EZ32H */
