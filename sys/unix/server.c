#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>

#include <event2/event.h>

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
  struct sockaddr_in addr = {};

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

