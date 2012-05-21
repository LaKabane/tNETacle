#pragma once
#ifndef TNTSOCKET_UUQ1C5JM
#define TNTSOCKET_UUQ1C5JM

#ifdef Windows

#define WIN32_LEAN_AND_MEAN
#include <Winsock2.h>

#else

#include <sys/types.h>
#include <sys/socket.h>

#endif /*Windows*/

enum tnt_socket_proto {
  IPv4 = AF_INET,
  IPv6 = AF_INET6,
  ANY = AF_UNSPEC,
};

#include <event2/util.h>

evutil_socket_t tnt_tcp_socket(enum tnt_socket_proto);
evutil_socket_t tnt_udp_socket(enum tnt_socket_proto);


#endif /* end of include guard: TNTSOCKET_UUQ1C5JM */
