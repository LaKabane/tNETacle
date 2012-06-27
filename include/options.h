/*
 * Copyright (c) 2012 Tristan Le Guern <leguern AT medu DOT se>
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
#include <arpa/inet.h>

#ifndef TNT_OPTIONS_H_ 
#define TNT_OPTIONS_H_ 

#define VECTOR_TYPE struct sockaddr
#define VECTOR_PREFIX sockaddr
#include "vector.h"
#undef VECTOR_TYPE
#undef VECTOR_PREFIX

struct options {
    int tunnel;                    /* Tunnel type: layer 2 or 3 */
    int tunnel_index;              /* Force the device instance number */
    int mode;                      /* Server mode: router, switch or hub */

    int debug;                     /* If true debug is allowed */
    int compression;               /* If true compression is allowed */
    int encryption;                /* If true encryption is allowed */

    int ports[TNETACLE_MAX_PORTS]; /* Port number to listen on */
    int addr_family;               /* Address family used by the server */
    /* Addresses on which the server listens */
    struct vector_sockaddr listen_addrs;
    /* Addresses of others tNETacle daemons */
    struct vector_sockaddr peer_addrs;
    char *addr;                    /* Address on the VPN */

    const char *key_path;

    /* Parsing stuff */
    const unsigned char *last_map_key;
    int   last_map_key_len;
};

enum {
    TNT_TUNMODE_TUNNEL,
    TNT_TUNMODE_ETHERNET
};

enum {
    TNT_DAEMONMODE_ROUTER,
    TNT_DAEMONMODE_SWITCH,
    TNT_DAEMONMODE_HUB
};

#endif

