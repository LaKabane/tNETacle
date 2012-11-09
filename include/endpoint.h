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
#ifndef ENDPOINT_DKUGVTSJ
#define ENDPOINT_DKUGVTSJ

#include "networking.h"

struct endpoint
{
    struct sockaddr_storage addr;
    socklen_t               addrlen;
};

void endpoint_init(struct endpoint *e,
              struct sockaddr *addr,
              socklen_t addrlen);

struct endpoint *
endpoint_new(void);

struct endpoint *
endpoint_clone(struct endpoint *src);

void
endpoint_copy(struct endpoint *dst,
              struct endpoint *src);

int
endpoint_set_ipport(struct endpoint *e,
                    char const *ip);

int
endpoint_port(struct endpoint *e);

struct sockaddr *
endpoint_addr(struct endpoint *e);

struct sockaddr *
endpoint_addrlen(struct endpoint *e);

void
endpoint_set_port(struct endpoint *e,
                  int port);

void
endpoint_assign_sockname(int socket,
                  struct endpoint *e);

char const *
endpoint_presentation(struct endpoint *e);

#endif /* end of include guard: ENDPOINT_DKUGVTSJ */
