/*
 * Copyright (c) 2011,2012 Tristan Le Guern <leguern AT medu DOT se>
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
#include <sys/stat.h>
#if defined Unix
# include <sys/socket.h>
# include <sys/mman.h>
#else
# include <Windows.h>
#endif

#if defined Unix
# include <netdb.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#else
# include <winsock2.h>
# include <Ws2tcpip.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <event2/util.h>
#include <yajl/yajl_parse.h>

#ifdef WIN32
# include "winstrtonum.h"
# define __func__ __FUNCTION__
# define alloca _alloca
# define snprintf _snprintf
#endif

#include "tnetacle.h"
#include "options.h"

extern int debug;
struct options server_options;

static void
init_options(struct options *opt) {
    unsigned int i;

    (void)memset(opt, '\0', sizeof(*opt));

    opt->tunnel = (int)TNT_TUNMODE_TUNNEL;
    opt->tunnel_index = -1;
    opt->mode = TNT_DAEMONMODE_ROUTER;

    opt->debug = 0;
    opt->compression = 1;
    opt->encryption = 1;

    for (i = 0; i < TNETACLE_MAX_PORTS; ++i)
        opt->ports[i] = -1;
    opt->ports[0] = TNETACLE_DEFAULT_PORT;
    opt->addr_family = AF_UNSPEC;
    opt->listen_addrs = NULL;
    opt->listen_addrs_num = 0;

    opt->peer_addrs = NULL;
    opt->peer_addrs_num = 0;

    opt->addr = NULL;

    opt->key_path= NULL;

    opt->last_map_key = NULL;
    opt->last_map_key_len = 0;
}

static int
add_sockaddr(struct sockaddr *sock, size_t *index, struct sockaddr *bufaddr) {
    size_t newsize;
    struct sockaddr *newsock;

    newsize = *index + 1;
    if ((newsock = realloc(sock, newsize)) == NULL) {
        free(sock);
        sock = NULL;
        *index = 0;
        return -1;
    }
    sock = newsock;
    *(sock + *index) = *bufaddr;
    *index = newsize;
    return 0;
}

static int
add_sockaddr_buf(struct sockaddr *sock, size_t *index, char *bufaddr) {
    int socklen;
    size_t newsize;
    struct sockaddr *newsock;
    struct sockaddr out;

    (void)memset(&out, '\0', sizeof out);
   
    socklen = sizeof(struct sockaddr);
    /* TODO: Sanity check with socklen */
    evutil_parse_sockaddr_port(bufaddr, &out, &socklen);
    
    newsize = *index + 1;
    if ((newsock = realloc(sock, newsize)) == NULL) {
        free(sock);
        sock = NULL;
        *index = 0;
        return -1;
    }
    sock = newsock;
    *(sock + *index) = out;
    *index = newsize;
    return 0;
}

int yajl_null(void *ctx) {
    (void)ctx;
    return -1;
}

int yajl_boolean(void *ctx, int val) {
    const char *map = server_options.last_map_key;
    const int map_len = server_options.last_map_key_len;

    (void)ctx;
    if (map == NULL)
        return -1;

    if (strncmp("Compression", map, map_len) == 0) {
        server_options.compression = val;
    } else if (strncmp("Encryption", map, map_len) == 0) {
        server_options.encryption = val;
    } else if (strncmp("Debug", map, map_len) == 0) {
        server_options.debug = val;
    } else {
        char *s;

        s = alloca(map_len);
        (void)memcpy(s, map, map_len);
        s[map_len] = '\0';
        fprintf(stderr, "%s: unknown boolean\n", s);
        return -1;
    }
    server_options.last_map_key = NULL;
    server_options.last_map_key_len = 0;
    return 1;
}

int yajl_integer(void *ctx, long long val) {
    (void)ctx;
    (void)val;
    return -1;
}

int yajl_double(void *ctx, double val) {
    (void)ctx;
    (void)val;
    return -1;
}

int yajl_number(void *ctx, const char *num, size_t len) {
    const char *map = server_options.last_map_key;
    const int map_len = server_options.last_map_key_len;
    const char *errstr;
    long long ret;
    char nptr[20];

    if (map == NULL)
        return -1;

    if (len > 20) {
        fprintf(stderr, "%s l%i, buffer not long enough\n",
          __func__, __LINE__);
        return -1;
    }

    (void)memcpy(nptr, num, len);
    ret = strtonum(num, 0, 256, &errstr);
    if (errstr == NULL) {
        fprintf(stderr, "TunnelIndex: %s\n", errstr);
        return -1;
    }

    if (strncmp("TunnelIndex", map, map_len) == 0) {
        server_options.tunnel_index = ret;
    } else if (strncmp("Port", map, map_len) == 0) {
        unsigned int i;

        for (i = 0; server_options.ports[i] != -1; ++i)
            ;
        server_options.ports[i] = ret;
    } else {
       char *s;

       s = alloca(map_len);
       (void)memcpy(s, map, map_len);
       s[map_len] = '\0';

       fprintf(stderr, "%s: unknown integer\n", s);
        return -1;
    }
    (void)ctx;
    server_options.last_map_key = NULL;
    server_options.last_map_key_len = 0;
    return 1;
}

int yajl_string(void *ctx, const unsigned char *str, size_t len) {
    const char *map = server_options.last_map_key;
    const int map_len = server_options.last_map_key_len;

    (void)ctx;
    if (map == NULL)
        return -1;

    if (strncmp("Address", map, map_len) == 0) {
        /* XXX: Address validation */
        server_options.addr = strndup(str, len);
        if (server_options.addr == NULL) {
            perror(__func__);
            return -1;
        }
    } else if (strncmp("AddressFamily", map, map_len) == 0) {
        if (strncmp("inet6", str, len) == 0) {
            server_options.addr_family = AF_INET;
        } else if (strncmp("inet", str, len) == 0) {
            server_options.addr_family = AF_INET6;
        } else if (strncmp("any", str, len) == 0) {
            server_options.addr_family = AF_UNSPEC;
        } else {
            fprintf(stderr, "AddressFamily: bad value, should be "
              "\"inet6\", \"inet\" or \"any\"\n");
            return -1;
        }
    } else if (strncmp("Mode", map, map_len) == 0) {
        if (strncmp("router", str, len) == 0) {
            server_options.tunnel = TNT_DAEMONMODE_ROUTER;
        } else if (strncmp("switch", str, len) == 0) {
            server_options.tunnel = TNT_DAEMONMODE_SWITCH;
        } else if (strncmp("hub", str, len) == 0) {
            server_options.tunnel = TNT_DAEMONMODE_HUB;
        } else {
            fprintf(stderr, "Mode: bad value, should be "
              "\"router\", \"switch\" or \"hub\"\n");
            return -1;
        }
    } else if (strncmp("Tunnel", map, map_len) == 0) {
         if (strncmp("point-to-point", str, len) == 0) {
            server_options.tunnel = TNT_TUNMODE_TUNNEL;
        } else if (strncmp("ethernet", str, len) == 0) {
            server_options.tunnel = TNT_TUNMODE_ETHERNET;
        } else {
            fprintf(stderr, "Tunnel: bad value, should be "
              "\"ethernet\" or \"point-to-point\"\n");
            return -1;
        }
    } else if (strncmp("PrivateKey", map, map_len) == 0) {
        /* XXX: Should we check for the existence of the key now ? */
        server_options.key_path = strndup(map, map_len);
        if (server_options.key_path == NULL) {
            perror(__func__);
            return -1;
        }
    } else if (strncmp("PeerAddress", map, map_len) == 0) {
        char bufaddr[45]; /* IPv6 with IPv4 tunnelling */
        (void)memset(bufaddr, '\0', sizeof bufaddr);
        (void)memcpy(bufaddr, str, len);

        add_sockaddr_buf(server_options.peer_addrs,
          &server_options.peer_addrs_num, bufaddr);
    } else if (strncmp("ListenAddress", map, map_len) == 0) {
        char bufaddr[45]; /* IPv6 with IPv4 tunnelling */
        (void)memset(bufaddr, '\0', sizeof bufaddr);
        (void)memcpy(bufaddr, str, len);

        if (strncmp("any", str, len) == 0) {
            /* Feed local addresses */
            struct sockaddr_in sin;
            struct sockaddr_in6 sin6;

            sin.sin_family = AF_INET;
            sin.sin_port = htons(TNETACLE_DEFAULT_PORT);
            sin.sin_addr.s_addr = inet_addr(0);
            sin.sin_family = AF_INET6;
            sin.sin_port = htons(TNETACLE_DEFAULT_PORT);
            sin.sin_addr.s_addr = inet_addr(0);

            add_sockaddr(server_options.listen_addrs,
              &server_options.listen_addrs_num, (struct sockaddr*)&sin);
            add_sockaddr(server_options.listen_addrs,
              &server_options.listen_addrs_num, (struct sockaddr*)&sin6);
        } else
            add_sockaddr_buf(server_options.listen_addrs,
              &server_options.listen_addrs_num, bufaddr);
    } else {
        char *s;

        s = alloca(map_len);
        (void)memcpy(s, map, map_len);
        s[map_len] = '\0';
        fprintf(stderr, "%s: unknown variable\n", s);
        return -1;
    }
    server_options.last_map_key = NULL;
    server_options.last_map_key_len = 0;
    return 1;
}

int yajl_start_map(void *ctx) {
    (void)ctx;
    return -1;
}

int yajl_map_key(void *ctx, const unsigned char *key, size_t len) {
    (void)ctx;
    server_options.last_map_key = key;
    server_options.last_map_key_len = len;
    return 1;
}

int yajl_end_map(void *ctx) {
    (void)ctx;
    return -1;
}

int yajl_start_array(void *ctx) {
    (void)ctx;
    return -1;
}

int yajl_end_array(void *ctx) {
    (void)ctx;
    return -1;
}

static yajl_callbacks callbacks = {
	yajl_null,
	yajl_boolean,
	yajl_integer,
	yajl_double,
	yajl_number,
	yajl_string,
	yajl_start_map,
	yajl_map_key,
	yajl_end_map,
	yajl_start_array,
	yajl_end_array
};

int
tnt_parse_buf(char *p, size_t size) {
	yajl_handle parse;
	yajl_status status;

    /* Overwrite previous configuration in case of SIGHUP */
    init_options(&server_options);

	parse = yajl_alloc(&callbacks, NULL, NULL);
	yajl_config(parse, yajl_allow_comments, 1);

    /* yajl might be feeded a bit at a time, but it also works this way */
	status = yajl_parse(parse, p, size);
	if (status != yajl_status_ok) {
		char *err;
		
		err = yajl_get_error(parse, 1, p, size);
		fprintf(stderr, "%s\n", err);
		yajl_free_error(parse, err);
	}

	status = yajl_complete_parse(parse);

	yajl_free(parse);

	if (status != yajl_status_ok) {
		return -1;
	}

    debug = server_options.debug;
    return 0;
}

