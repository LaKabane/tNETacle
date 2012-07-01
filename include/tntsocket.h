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
#ifndef TNTSOCKET_UUQ1C5JM
#define TNTSOCKET_UUQ1C5JM

#ifdef Windows
# define WIN32_LEAN_AND_MEAN
# include <Winsock2.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
#endif /*Windows*/

#include <event2/util.h>

enum tnt_socket_proto {
  IPv4 = AF_INET,
  IPv6 = AF_INET6,
  ANY = AF_UNSPEC,
};

evutil_socket_t tnt_tcp_socket(enum tnt_socket_proto);
evutil_socket_t tnt_udp_socket(enum tnt_socket_proto);

#endif /* end of include guard: TNTSOCKET_UUQ1C5JM */
