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

#ifndef ENDPOINT_DKUGVTSJ
# define ENDPOINT_DKUGVTSJ

struct endpoint {
    struct sockaddr_storage addr;
    socklen_t               addrlen;
};

void             endpoint_init(struct endpoint *, struct sockaddr const *,
                               socklen_t const);
struct endpoint *endpoint_new(void);
struct endpoint *endpoint_clone(struct endpoint const *);
void             endpoint_copy(struct endpoint *, struct endpoint const *);
int              endpoint_set_ipport(struct endpoint *, char const *);
int              endpoint_port(struct endpoint const *);
struct sockaddr *endpoint_addr(struct endpoint const *);
socklen_t        endpoint_addrlen(struct endpoint const *);
void             endpoint_set_port(struct endpoint *, int port);
void             endpoint_assign_sockname(int, struct endpoint *);
char const      *endpoint_presentation(struct endpoint const *);
int              endpoint_cmp(struct endpoint const *,
                              struct endpoint const *);

#endif /* end of include guard: ENDPOINT_DKUGVTSJ */
