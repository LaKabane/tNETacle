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

#include "tntsocket.h"

evutil_socket_t tnt_tcp_socket(enum tnt_socket_proto p)
{
    int sock;

    sock = socket(p, SOCK_STREAM, IPPROTO_TCP);
    if (sock != -1)
    {
	return sock;
    }
    return -1;
}

evutil_socket_t tnt_udp_socket(enum tnt_socket_proto p)
{
    int sock;

    sock = socket(p, SOCK_DGRAM, IPPROTO_UDP);
    if (sock != -1)
    {
	return sock;
    }
    return -1;
}
