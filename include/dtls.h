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

#ifndef DTLS_B4BZSCI2
#define DTLS_B4BZSCI2

struct udp_peer;
struct udp;

int dtls_new_peer(SSL_CTX *ctx,
                  struct udp_peer *p);

SSL_CTX *create_udp_ctx(void);

ssize_t dtls_recvfrom(int sockfd,
              void *buf,
              size_t len,
              int flags,
              struct sockaddr *addr,
              int *socklen,
              struct udp *udp,
              void *async_ctx);

ssize_t dtls_sendto(int sockfd,
                    void const *buf,
                    size_t len,
                    int flags,
                    struct sockaddr const *addr,
                    int socklen,
                    struct udp_peer *perr,
                    void *async_ctx);

ssize_t dtls_do_handshake(int fd,
                          struct udp *udp,
                          struct udp_peer *peer,
                          void *async_ctx);

#endif /* end of include guard: DTLS_B4BZSCI2 */
