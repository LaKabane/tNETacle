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
# include <netdb.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/mman.h>
#else
# define WIN32_LEAN_AND_MEAN
# include <Winsock2.h>
# include <Windows.h>
# include <Ws2tcpip.h>
# include "wincompat.h"
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <event2/util.h>
#include <yajl/yajl_parse.h>

#include "tnetacle.h"
#include "options.h"

#define VECTOR_TYPE struct cfg_sockaddress
#define VECTOR_PREFIX sockaddr
#define DEFAULT_ALLOC_SIZE 2
#define VECTOR_NON_STATIC
#include "vector.h"

extern int debug;
struct options serv_opts;

struct ctx {
    const unsigned char *map;
    int   len;
};

void
init_options(struct options *opt) {
    unsigned int i;

    (void)memset(opt, '\0', sizeof(*opt));

    opt->tunnel = (int)TNT_TUNMODE_TUNNEL;
    opt->tunnel_index = -1;
    opt->mode = TNT_DAEMONMODE_ROUTER;

    opt->debug = 0;
    opt->compression = 1;
    opt->encryption = 1;

    for (i = 0; i < TNETACLE_MAX_PORTS; ++i) {
        opt->ports[i] = -1;
        opt->cports[i] = -1;
    }

    opt->addr_family = AF_UNSPEC;
    serv_opts.listen_addrs = v_sockaddr_new();
	serv_opts.client_addrs = v_sockaddr_new();
    serv_opts.peer_addrs = v_sockaddr_new();

    opt->addr = NULL;

    opt->key_path= NULL;
}

static int
add_sockaddr(struct vector_sockaddr *vsp, char *bufaddr) {
    struct cfg_sockaddress out;

    (void)memset(&out, 0, sizeof out);

    /* Take the size from the sockaddr_storage*/
    out.len = sizeof(out.sockaddr);
    /* TODO: Sanity check with socklen */
    if (evutil_parse_sockaddr_port(bufaddr, (struct sockaddr *)&out.sockaddr,
      &out.len) == -1) {
        (void)fprintf(stderr, "%s: not a valid IP address\n", bufaddr);
        return -1;
    }
    /* We now have a good sockaddr_storage, with the right length*/
    v_sockaddr_push(vsp, &out);
    return 0;
}

static int
add_listen_addrs_ports(int family, int *ports) {
    unsigned int i;
    struct cfg_sockaddress tmp_store;
    struct sockaddr_in *sin = (struct sockaddr_in *)&tmp_store.sockaddr;
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&tmp_store.sockaddr;

    for (i = 0; i < TNETACLE_MAX_PORTS && ports[i] != -1; ++i) {
        (void)memset(&tmp_store, 0, sizeof tmp_store);
    
        if (family == AF_INET || family == AF_UNSPEC) {
            sin->sin_family = AF_INET;
            sin->sin_port = htons(ports[i]);
            if (inet_pton(AF_INET, TNETACLE_DEFAULT_LISTEN_IPV4,
              &sin->sin_addr.s_addr) == -1)
                return -1;
            tmp_store.len = sizeof *sin;
            v_sockaddr_push(serv_opts.listen_addrs, &tmp_store);
            if (debug == 1)
                fprintf(stderr, "ListenAddr: Added %s:%i\n",
                  TNETACLE_DEFAULT_LISTEN_IPV4, ports[i]);
        }
        if (family == AF_INET6 || family == AF_UNSPEC) {
            sin6->sin6_family = AF_INET6;
            sin6->sin6_port = htons(ports[i]);
            if (inet_pton(AF_INET6, TNETACLE_DEFAULT_LISTEN_IPV6,
              &sin6->sin6_addr.s6_addr) == -1)
                return -1;
            tmp_store.len = sizeof *sin6;
            v_sockaddr_push(serv_opts.listen_addrs, &tmp_store);
            if (debug == 1)
                fprintf(stderr, "ListenAddr: Added [%s]:%i\n",
                  TNETACLE_DEFAULT_LISTEN_IPV6, ports[i]);
        }
    }
}

static int
add_client_addrs_ports(int family, int *ports) {
    unsigned int i;
    struct cfg_sockaddress tmp_store;
    struct sockaddr_in *sin = (struct sockaddr_in *)&tmp_store.sockaddr;
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&tmp_store.sockaddr;

    for (i = 0; i < TNETACLE_MAX_PORTS && ports[i] != -1; ++i) {
        (void)memset(&tmp_store, 0, sizeof tmp_store);

        if (family == AF_INET || family == AF_UNSPEC) {
            sin->sin_family = AF_INET;
            sin->sin_port = htons(ports[i]);
            if (inet_pton(AF_INET, TNETACLE_DEFAULT_LISTEN_IPV4,
              &sin->sin_addr.s_addr) == -1)
                return -1;
            tmp_store.len = sizeof *sin;
            v_sockaddr_push(serv_opts.client_addrs, &tmp_store);
            if (debug == 1)
                fprintf(stderr, "ListenClient: Added %s:%i\n",
                  TNETACLE_DEFAULT_LISTEN_IPV4, ports[i]);
        }
        if (family == AF_INET6 || family == AF_UNSPEC) {
            sin6->sin6_family = AF_INET6;
            sin6->sin6_port = htons(ports[i]);
            if (inet_pton(AF_INET6, TNETACLE_DEFAULT_LISTEN_IPV6,
              &sin6->sin6_addr.s6_addr) == -1)
                return -1;
            tmp_store.len = sizeof *sin6;
            v_sockaddr_push(serv_opts.client_addrs, &tmp_store);
            if (debug == 1)
                fprintf(stderr, "ListenClient: Added [%s]:%i\n",
                  TNETACLE_DEFAULT_LISTEN_IPV6, ports[i]);
        }
    }
}

int yajl_null(void *ctx) {
    (void)ctx;
    return -1;
}

int yajl_boolean(void *lctx, int val) {
    struct ctx *ctx = lctx;

    if (ctx->map == NULL)
        return -1;

    if (strncmp("Compression", ctx->map, ctx->len) == 0) {
        serv_opts.compression = val;
    } else if (strncmp("Encryption", ctx->map, ctx->len) == 0) {
        serv_opts.encryption = val;
    } else if (strncmp("Debug", ctx->map, ctx->len) == 0) {
        serv_opts.debug = val;
    } else {
        char *s;

        s = alloca(ctx->len);
        (void)memcpy(s, ctx->map, ctx->len);
        s[ctx->len] = '\0';
        fprintf(stderr, "%s: unknown boolean\n", s);
        return -1;
    }
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

int yajl_number(void *lctx, const char *num, size_t len) {
    char *errstr;
    long ret;
    char nptr[20];
    struct ctx *ctx = lctx;

    if (ctx->map == NULL)
        return -1;

    if (len > 20) {
        fprintf(stderr, "%s l%i, buffer not long enough\n",
          __func__, __LINE__);
        return -1;
    }

    (void)memset(nptr, 0, sizeof nptr);
    (void)memcpy(nptr, num, len);

    ret = strtol(nptr, &errstr, 10);
    if (*errstr != '\0' || ret < 0 || ret > 65535) {
        fprintf(stderr, "[%s] TunnelIndex or Port has not a valid value\n",
	  nptr);
        return -1;
    }

    if (strncmp("TunnelIndex", ctx->map, ctx->len) == 0) {
        serv_opts.tunnel_index = ret;
    } else if (strncmp("Port", ctx->map, ctx->len) == 0) {
        unsigned int i;

        for (i = 0; i < TNETACLE_MAX_PORTS && serv_opts.ports[i] != -1; ++i)
            ;
        serv_opts.ports[i] = ret;
    } else if (strncmp("ClientPort", ctx->map, ctx->len) == 0) {
        unsigned int i;

        for (i = 0; i < TNETACLE_MAX_PORTS && serv_opts.cports[i] != -1; ++i)
            ;
        serv_opts.cports[i] = ret;
    } else {
       char *s;

       s = alloca(ctx->len);
       (void)memcpy(s, ctx->map, ctx->len);
       s[ctx->len] = '\0';

       fprintf(stderr, "%s: unknown integer\n", s);
        return -1;
    }
    (void)ctx;
    return 1;
}

int yajl_string(void *lctx, const unsigned char *str, size_t len) {
    struct ctx *ctx = lctx;

    if (ctx->map == NULL)
        return -1;

    if (strncmp("Address", ctx->map, ctx->len) == 0) {
        /* XXX: Address validation */
        serv_opts.addr = strndup(str, len);
        if (serv_opts.addr == NULL) {
            perror(__func__);
            return -1;
        }
    } else if (strncmp("AddressFamily", ctx->map, ctx->len) == 0) {
        if (strncmp("inet6", str, len) == 0) {
            serv_opts.addr_family = AF_INET;
        } else if (strncmp("inet", str, len) == 0) {
            serv_opts.addr_family = AF_INET6;
        } else if (strncmp("any", str, len) == 0) {
            serv_opts.addr_family = AF_UNSPEC;
        } else {
            fprintf(stderr, "AddressFamily: bad value, should be "
              "\"inet6\", \"inet\" or \"any\"\n");
            return -1;
        }
    } else if (strncmp("Mode", ctx->map, ctx->len) == 0) {
        if (strncmp("router", str, len) == 0) {
            serv_opts.tunnel = TNT_DAEMONMODE_ROUTER;
        } else if (strncmp("switch", str, len) == 0) {
            serv_opts.tunnel = TNT_DAEMONMODE_SWITCH;
        } else if (strncmp("hub", str, len) == 0) {
            serv_opts.tunnel = TNT_DAEMONMODE_HUB;
        } else {
            fprintf(stderr, "Mode: bad value, should be "
              "\"router\", \"switch\" or \"hub\"\n");
            return -1;
        }
    } else if (strncmp("Tunnel", ctx->map, ctx->len) == 0) {
         if (strncmp("point-to-point", str, len) == 0) {
            serv_opts.tunnel = TNT_TUNMODE_TUNNEL;
        } else if (strncmp("ethernet", str, len) == 0) {
            serv_opts.tunnel = TNT_TUNMODE_ETHERNET;
        } else {
            fprintf(stderr, "Tunnel: bad value, should be "
              "\"ethernet\" or \"point-to-point\"\n");
            return -1;
        }
    } else if (strncmp("PrivateKey", ctx->map, ctx->len) == 0) {
        /* XXX: Should we check for the existence of the key now ? */
        serv_opts.key_path = strndup(str, len);
        if (serv_opts.key_path == NULL) {
            perror(__func__);
            return -1;
        }
    } else if (strncmp("CertFile", ctx->map, ctx->len) == 0) {
        /* XXX: Should we check for the existence of the key now ? */
        serv_opts.cert_path = strndup(str, len);
        if (serv_opts.cert_path == NULL) {
            perror(__func__);
            return -1;
        }
    } else if (strncmp("PeerAddress", ctx->map, ctx->len) == 0) {
        char bufaddr[INET6_ADDRSTRLEN]; /* IPv6 with IPv4 tunnelling */
        (void)memset(bufaddr, '\0', sizeof bufaddr);
        (void)memcpy(bufaddr, str, len);

        add_sockaddr(serv_opts.peer_addrs, bufaddr);
    } else if (strncmp("ListenAddress", ctx->map, ctx->len) == 0) {
        char bufaddr[INET6_ADDRSTRLEN]; /* IPv6 with IPv4 tunnelling */
        (void)memset(bufaddr, '\0', sizeof bufaddr);
        (void)memcpy(bufaddr, str, len);

        if (strncmp("any", str, len) == 0) {
	    add_listen_addrs_ports(serv_opts.addr_family, serv_opts.ports);
        } else
            add_sockaddr(serv_opts.listen_addrs, bufaddr);
        add_client_addrs_ports(serv_opts.addr_family, serv_opts.cports);
    } else {
        char *s;

        s = alloca(ctx->len);
        (void)memcpy(s, ctx->map, ctx->len);
        s[ctx->len] = '\0';
        fprintf(stderr, "%s: unknown variable\n", s);
        return -1;
    }
    return 1;
}

int yajl_start_map(void *ctx) {
    (void)ctx;
    return -1;
}

int yajl_map_key(void *lctx, const unsigned char *key, size_t len) {
    struct ctx *ctx = lctx;

    ctx->map = key;
    ctx->len = len;
    return 1;
}

int yajl_end_map(void *lctx) {
    struct ctx *ctx = lctx;

    ctx->map = NULL;
    ctx->len = 0;
    return -1;
}

int yajl_start_array(void *ctx) {
    (void)ctx;
    return -1;
}

int yajl_end_array(void *lctx) {
    struct ctx *ctx = lctx;

    ctx->map = NULL;
    ctx->len = 0;
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
	struct ctx ctx;

        /* Overwrite previous configuration in case of SIGHUP */
        init_options(&serv_opts);
	ctx.map = NULL;
	ctx.len = 0;

	parse = yajl_alloc(&callbacks, NULL, &ctx);
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
		const char *err = yajl_status_to_string(status);
		fprintf(stderr, "%s\n", err);
		return -1;
	}

    debug = serv_opts.debug;
    if (serv_opts.ports[0] == -1)
        serv_opts.ports[0] = TNETACLE_DEFAULT_PORT;
    if (serv_opts.cports[0] == -1)
        serv_opts.cports[0] = CLIENT_DEFAULT_PORT;
    return 0;
}

