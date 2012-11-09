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

#include "endpoint.h"
#include "subset.h"
#include <string.h>
#include <event2/util.h>

void
endpoint_init(struct endpoint *e,
              struct sockaddr *addr,
              socklen_t addrlen)
{
    memcpy(&e->addr, addr, addrlen);
    e->addrlen = addrlen;
}

struct endpoint *
endpoint_new(void)
{
    struct endpoint *e = tnt_new(struct endpoint);

    return e;
}

int
endpoint_set_ipport(struct endpoint *e,
                    char const *ip)
{
    return evutil_parse_sockaddr_port(ip,
                                      (struct sockaddr *)&e->addr,
                                      &e->addrlen);
}

int
endpoint_port(struct endpoint *e)
{
    int port;

    switch (e->addr.ss_family)
    {
        case AF_INET:
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)&e->addr;

                port = sin->sin_port;
                break;
            }
        case AF_INET6:
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&e->addr;

                port = sin6->sin6_port;
                break;
            }
    }
    return port;
}

void
endpoint_set_port(struct endpoint *e,
                  int port)
{
    switch (e->addr.ss_family)
    {
        case AF_INET:
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)&e->addr;

                sin->sin_port = port;
                break;
            }
        case AF_INET6:
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&e->addr;

                sin6->sin6_port = port;
                break;
            }
    }
}

struct sockaddr *
endpoint_addr(struct endpoint *e)
{
    return (struct sockaddr *)&e->addr;
}

struct sockaddr *
endpoint_addrlen(struct endpoint *e)
{
    return (struct sockaddr *)&e->addrlen;
}

void
endpoint_copy(struct endpoint *dst,
              struct endpoint *src)
{
    memcpy(dst, src, src->addrlen);
    dst->addrlen = src->addrlen;
}

struct endpoint *
endpoint_clone(struct endpoint *src)
{
    struct endpoint *new = tnt_new(struct endpoint);

    endpoint_copy(new, src);
    return new;
}

void
endpoint_assign_sockname(int socket,
                  struct endpoint *e)
{
    getsockname(socket, endpoint_addr(e), &e->addrlen);
}

char const *
endpoint_presentation(struct endpoint *e)
{
    static char name[INET6_ADDRSTRLEN];

    switch (e->addr.ss_family)
    {
        case AF_INET:
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)&e->addr;
                char tmp[INET_ADDRSTRLEN];

                evutil_inet_ntop(AF_INET, &sin->sin_addr, tmp, sizeof tmp);
                evutil_snprintf(name, sizeof name, "%s:%d", tmp, ntohs(sin->sin_port));
                return name;
            }
        case AF_INET6:
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&e->addr;
                char tmp[INET6_ADDRSTRLEN];

                evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, tmp, sizeof tmp);
                evutil_snprintf(name, sizeof name, "%s:%d", tmp, ntohs(sin6->sin6_port));
                return name;
            }
    }
    return name;
}
