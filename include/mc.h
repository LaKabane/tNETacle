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

#include "networking.h"

/*
 * We include this one after networking.h because openssl includes windows.h
 * leading to a redifinition of most of the wsaapi symbols on Windows.
 * Seriously, fuck you OpenSSL.
 */
#include <openssl/ssl.h> /*Can not forward declare SSL types..*/
#include "endpoint.h"

#ifndef MC_ENDPOINT_JU2N66SJ
# define MC_ENDPOINT_JU2N66SJ

struct bufferevent;
struct event_base;
struct sockaddr;
struct server;
struct frame;
struct udp;

enum tls_flags {
    TLS_ENABLE = (1 << 0),
    TLS_DISABLE = (1 << 1),
};

struct mc {
    struct peer {
        struct sockaddr *address;
        socklen_t        len;
    } p;
    struct bufferevent  *bev;
    int                  ssl_flags;
    int                  tunel;
};

int          mc_init(struct mc *, struct event_base *, int, struct sockaddr *,
                     socklen_t, SSL_CTX *);
void         mc_close(struct mc *);
int          mc_add_raw_data(struct mc *, void *, size_t);
int          mc_hello(struct mc *, struct udp *);
int          mc_establish_tunnel(struct mc *, struct udp *);
struct mc   *mc_peer_accept(struct server *, struct event_base *,
                            struct sockaddr *, int, int);
struct mc   *mc_peer_connect(struct server *, struct event_base *,
                             struct sockaddr *, int);
int          mc_established(struct server *, struct sockaddr *, int);
int          mc_pending(struct server *, struct sockaddr *, int);

/* used for debug, and print a mc */
char        *mc_presentation(struct mc *, char *, int);
char        *address_presentation(struct sockaddr *, socklen_t, char *, int);

#endif /* end of include guard: MC_ENDPOINT_JU2N66SJ */
