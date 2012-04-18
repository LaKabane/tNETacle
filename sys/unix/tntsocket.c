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
