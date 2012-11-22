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

#include <openssl/ssl.h>
#include <openssl/bio.h>

#include "networking.h"
#include "coro.h"
#include "tntsched.h"
#include "endpoint.h"

#ifndef UDP_US4EZ32H
# define UDP_US4EZ32H

enum udp_ssl_flags
{
    DTLS_ENABLE = (1 << 0),
    DTLS_DISABLE = (1 << 1),
    DTLS_CLIENT = (1 << 2),
    DTLS_SERVER = (1 << 3),
    DTLS_CONNECTED = (1 << 4),
};

struct server;
struct frame;
struct sockaddr;
struct event;
struct vector_frame;

struct udp_peer
{
    struct endpoint         peer_addr;
    BIO                     *bio;
    BIO                     *_bio_backend;
    SSL                     *ssl;
    enum udp_ssl_flags      ssl_flags;
};

# define VECTOR_TYPE struct udp_peer
# define VECTOR_PREFIX udp
# include "vector.h"

struct udp
{
    int                     fd;
    SSL_CTX                 *ctx;
    struct fiber            *udp_recv_fib;
    struct fiber            *udp_brd_fib;
    struct vector_udp       *udp_peers;
    struct endpoint         udp_endpoint;
};

struct udp      *server_udp_new(struct server *, struct endpoint *);
int              server_udp_init(struct server *, struct udp *, struct endpoint *);
void             server_udp_launch(struct udp *);
void             server_udp_exit(struct udp *);
void             server_udp(void *);
void             server_dtls(void *);

struct udp_peer *udp_register_new_peer(struct udp *, struct endpoint *,int);
void             forward_udp_frame_to_other_peers(void *, struct udp *, struct frame *, struct sockaddr *, socklen_t);
void             broadcast_udp_to_peers(struct server *);
int              frame_recvfrom(void *, int, struct frame *, struct sockaddr *, socklen_t *);
unsigned short   udp_get_port(struct udp *);

#endif /* end of include guard: UDP_US4EZ32H */
