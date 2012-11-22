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

#ifdef Windows
# define WIN32_LEAN_AND_MEAN
# include <Winsock2.h>
# include "wincompat.h"
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
#endif /*Windows*/

#include <event2/util.h>

#ifndef TNTSOCKET_UUQ1C5JM
# define TNTSOCKET_UUQ1C5JM

evutil_socket_t tnt_tcp_socket(sa_family_t);
evutil_socket_t tnt_udp_socket(sa_family_t);

#endif /* end of include guard: TNTSOCKET_UUQ1C5JM */
