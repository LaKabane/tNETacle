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

#ifndef UDP_US4EZ32H
#define UDP_US4EZ32H

struct server;
struct frame;
struct sockaddr;

int server_init_udp(struct sockaddr *addr,
                int len);

void forward_udp_frame_to_other_peers(struct server *s,
                                 struct frame *current_frame,
                                 struct sockaddr *current_sockaddr,
                                 unsigned int current_socklen);

void broadcast_udp_to_peers(struct server *s);

int frame_recvfrom(int fd,
                   struct frame *frame,
                   struct sockaddr *saddr,
                   unsigned int *socklen);

#endif /* end of include guard: UDP_US4EZ32H */
