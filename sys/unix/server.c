/*
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <event2/event.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

static void
listen_callback(evutil_socket_t fd, short events, void *args) {
  struct server *s = args;
  char buf[2048] = {0};
 
  printf("%s\n", __PRETTY_FUNCTION__);
  if (events & EV_READ)
  {
    struct sockaddr_in sockaddr;
    socklen_t addrlen = sizeof(sockaddr);
    size_t n;
    printf("%s::[EVENT READ]\n", __PRETTY_FUNCTION__);

    if ((n = recvfrom(fd, buf, 2048, 0,
                      (struct sockaddr *) &sockaddr, &addrlen)) > 0)
    {
      int devfd = event_get_fd(s->device);

      printf("%s::[READ %lu bytes]\n", __PRETTY_FUNCTION__, n);
      write(devfd, buf, n);
    }
  }
  if (events & EV_ET)
    printf("%s::[EVENT ET]\n", __PRETTY_FUNCTION__);

}

void
server_set_device(struct server *s, struct event *devent)
{
  s->device = devent;
  event_add(s->listen_socket, NULL);
}

int
server_init(struct server *s, struct event_base *evbase)
{
  int fd;
  struct sockaddr_in addr;

  (void)memset(&addr, '\0', sizeof addr);
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    return -1;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(4242);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    return -1;

  evutil_make_listen_socket_reuseable(fd);
  evutil_make_socket_nonblocking(fd);

  s->listen_socket = event_new(evbase, fd,
                    EV_READ | EV_WRITE | EV_ET | EV_PERSIST,
                    &listen_callback, s);

  return 0;
}

