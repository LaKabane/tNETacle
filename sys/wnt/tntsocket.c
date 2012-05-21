#include "tntsocket.h"

evutil_socket_t tnt_tcp_socket(enum tnt_socket_proto p)
{
    SOCKET sock;

    sock = WSASocket((int)p, SOCK_STREAM, IPPROTO_TCP,
		     /*ldProtocolInfo*/NULL, /*group Id*/0,
		     WSA_FLAG_OVERLAPPED);
    if (sock != INVALID_SOCKET)
    {
	return sock;
    }
    return -1;
}

evutil_socket_t tnt_udp_socket(enum tnt_socket_proto p)
{
    SOCKET sock;

    sock = WSASocket((int)p, SOCK_DGRAM, IPPROTO_UDP,
		     /*ldProtocolInfo*/NULL, /*group Id*/0,
		     WSA_FLAG_OVERLAPPED);
    if (sock != INVALID_SOCKET)
    {
	return sock;
    }
    return -1;
}
