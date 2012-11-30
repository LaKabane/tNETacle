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

#include <stdio.h>
#include <string.h>

#include <event2/util.h>

#include "endpoint.h"
#include "subset.h"
#include "log.h"

void
endpoint_init(struct endpoint *e,
              struct sockaddr const *addr,
              socklen_t const addrlen)
{
    memset(e, 0, sizeof(struct endpoint));
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
                                       (int *)&e->addrlen);
}

int
endpoint_port(struct endpoint const *e)
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
    return ntohs(port);
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

                sin->sin_port = htons(port);
                break;
            }
        case AF_INET6:
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&e->addr;

                sin6->sin6_port = htons(port);
                break;
            }
    }
}

struct sockaddr *
endpoint_addr(struct endpoint const *e)
{
    return (struct sockaddr *)&e->addr;
}

socklen_t
endpoint_addrlen(struct endpoint const *e)
{
    return e->addrlen;
}

void
endpoint_copy(struct endpoint *dst,
              struct endpoint const *src)
{
    memcpy(dst, src, src->addrlen);
    dst->addrlen = src->addrlen;
}

struct endpoint *
endpoint_clone(struct endpoint const *src)
{
    struct endpoint *new_end = tnt_new(struct endpoint);

    endpoint_copy(new_end, src);
    return new_end;
}

void
endpoint_assign_sockname(intptr_t socket,
                  struct endpoint *e)
{
    getsockname(socket, endpoint_addr(e), &e->addrlen);
}

char const *
endpoint_presentation(struct endpoint const *e)
{
    static char name[INET6_ADDRSTRLEN];

    switch (e->addr.ss_family)
    {
        case AF_INET:
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)&e->addr;
                char tmp[INET_ADDRSTRLEN];

                evutil_inet_ntop(AF_INET, &sin->sin_addr, tmp, sizeof tmp);
                evutil_snprintf(name, INET6_ADDRSTRLEN, "%s:%d", tmp, ntohs(sin->sin_port));
                break;
            }
        case AF_INET6:
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&e->addr;
                char tmp[INET6_ADDRSTRLEN];

                evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, tmp, sizeof tmp);
                evutil_snprintf(name, INET6_ADDRSTRLEN, "%s:%d", tmp, ntohs(sin6->sin6_port));
                break;
            }
        default:
            {
                log_warnx("[ENDPOINT] presentation doesn't handle protocol", e->addr.ss_family);
            }
    }
    return name;
}

int
endpoint_cmp(struct endpoint const *a,
             struct endpoint const *b)
{
    if (a->addrlen != b->addrlen)
        return  -1;
    return evutil_sockaddr_cmp(endpoint_addr(a), endpoint_addr(b), 1);
}
