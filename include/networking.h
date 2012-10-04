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

#ifndef NETWORKING_LLW6GBE3
#define NETWORKING_LLW6GBE3

#if defined Unix
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
#elif defined Windows
# include <WinSock2.h>
# include <WS2tcpip.h>
# include <io.h>
# define EAGAIN WSAEWOULDBLOCK
/* If we typedef socklen_t, this won't compile on Windows, so let it like this*/
# define socklen_t size_t
#endif


#endif /* end of include guard: NETWORKING_LLW6GBE3 */
