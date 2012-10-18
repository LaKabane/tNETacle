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


#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined Windows
# define ssize_t SSIZE_T
#endif

#if defined Unix
# include <unistd.h>
#endif

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "networking.h"
#include "udp.h"
#include "frame.h"
#include "device.h"

#include "tnetacle.h"
#include "options.h"
#include "mc.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"
#include "hexdump.h"
#include "wincompat.h"
#include "client.h"

#define VECTOR_TYPE struct mc
#define VECTOR_PREFIX mc
#define VECTOR_NON_STATIC
#include "vector.h"

#define VECTOR_TYPE struct evconnlistener*
#define VECTOR_PREFIX evl
#define VECTOR_TYPE_SCALAR
#define VECTOR_NON_STATIC
#include "vector.h"

#define VECTOR_TYPE char*
#define VECTOR_PREFIX cptr
#define DEFAULT_ALLOC_SIZE 4
#define VECTOR_TYPE_SCALAR
#include "vector.h"

extern struct options serv_opts;

static int
find_bev(struct mc const *a, void *ctx)
{
    struct bufferevent *bev = ctx;
    return a->bev == bev;
}

static int
find_udppeer(struct udp_peer const *a, void *ctx)
{
    struct sockaddr *s = ctx;
    return !evutil_sockaddr_cmp((struct sockaddr *)&a->addr, s, 0);
}

char *next_token(char *ptr, char **saveit, char const *delimit)
{
    char *tmp;

    if (ptr == NULL)
    {
        ptr = *saveit;
        if (ptr == NULL)
            return NULL;
    }
    else
        ptr += strspn(ptr, delimit);

    tmp = strpbrk(ptr, delimit);
    if (tmp != NULL) 
    {
        *tmp = '\0';
        *saveit = tmp + 1;
    } 
    else
        *saveit = NULL;

    return ptr;
}

struct vector_cptr *
split(char *line)
{
    char *ptr;
    char *saveptr = NULL;
    struct vector_cptr *tmp = v_cptr_new();

    for (ptr = next_token(line, &saveptr, " :");
         ptr != NULL;
         ptr = next_token(NULL, &saveptr, " :"))
    {
        v_cptr_push(tmp, ptr);
    }
    return tmp;
}

void
server_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = (struct server *)ctx;
    struct evbuffer *in = bufferevent_get_input(bev);
    struct evbuffer *out = bufferevent_get_output(bev);
    size_t len;
    char *line;

    /* Get rid of every input */
    (void)s;
    while ((line = evbuffer_readln(in, &len, EVBUFFER_EOL_CRLF)) != NULL)
    {
        struct vector_cptr *splited;
        char *cmd_name;

        splited = split(line);
        cmd_name = v_cptr_at(splited, 0);
        if (strncmp(cmd_name, "udp_port", strlen(cmd_name)) == 0)
        {
            char *s_udp_port = v_cptr_at(splited, 1);
            struct mc *mc = v_mc_find_if(s->peers, (void *)find_bev, bev);
            unsigned short port = atoi(s_udp_port);
            struct sockaddr_storage udp_addr;
            unsigned int socklen;
            char *s_status = NULL;

            memcpy(&udp_addr, mc->p.address, mc->p.len);
            if (udp_addr.ss_family == AF_INET)
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)&udp_addr;

                sin->sin_port = htons(port);
                socklen = sizeof *sin;
            }
            else if (udp_addr.ss_family == AF_INET6)
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&udp_addr;

                sin6->sin6_port = htons(port);
                socklen = sizeof *sin6;
            }
            if (splited->size > 1)
                s_status = v_cptr_at(splited, 2);
            if (s_status == NULL || strcmp(s_status, "ok") != 0)
            {
                evbuffer_add_printf(out, "udp_port:%d ok\r\n", udp_get_port(&s->udp)); 
            }
            udp_register_new_peer(&s->udp,
                                  (struct sockaddr *)&udp_addr,
                                  socklen,
                                  DTLS_ENABLE);
        }
        v_cptr_delete(splited);
        free(line);
    }
}

static void
dump_flags(short events)
{
    log_debug("[EVENT] [%s:%s:%s:%s:%s:%s]",
              (events & BEV_EVENT_EOF) ? "EOF" : "",
              (events & BEV_EVENT_READING) ? "READING" : "",
              (events & BEV_EVENT_WRITING) ? "WRITING" : "",
              (events & BEV_EVENT_CONNECTED) ? "CONNECTED" : "",
              (events & BEV_EVENT_TIMEOUT) ? "TIMEOUT" : "",
              (events & BEV_EVENT_ERROR) ? "ERROR" : "");
}

void
server_mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    struct server *s = (struct server *)ctx;

    dump_flags(events);
    if (events & BEV_EVENT_CONNECTED)
    {
        struct mc *mc;

        /*
         * If we received the notification that the connection is established,
         * then we move the corresponding struct mc from s->pending_peers to
         * s->peers.
         */

        mc = v_mc_find_if(s->pending_peers, (void *)find_bev, bev);
        if (mc != v_mc_end(s->pending_peers))
        {
            struct mc tmp;

            log_info("connexion established.");
            memcpy(&tmp, mc, sizeof(tmp));
            v_mc_erase(s->pending_peers, mc);
            v_mc_push(s->peers, &tmp);
            mc_hello(&tmp, &s->udp);
            //mc_establish_tunnel(&tmp, s->udp);
        }
    }
    else if (events & BEV_EVENT_EOF)
    {
        /* Disconnected */
        struct mc *mc;
        struct udp_peer *up;

        mc = v_mc_find_if(s->peers, (void *)find_bev, bev);
        if (mc != v_mc_end(s->peers))
        {
            char name[INET6_ADDRSTRLEN];
            struct sockaddr *sock = mc->p.address;

            up = v_udp_find_if(s->udp.udp_peers, find_udppeer, sock);
            if (up != v_udp_end(s->udp.udp_peers))
            {
                v_udp_erase(s->udp.udp_peers, up);
                log_debug("[%s] stop peering with %s",
                          (up->ssl_flags & DTLS_ENABLE) ? "DTLS" : "UDP",
                          address_presentation((struct sockaddr *)&up->addr,
                                               up->socklen,
                                               name, sizeof(name)));
            }
            log_debug("[META] stop the meta-connexion with %s",
                      mc_presentation(mc, name, sizeof(name)));
            mc_close(mc);
            v_mc_erase(s->peers, mc);
        }

    }
    else if (events & BEV_EVENT_ERROR)
    {
        struct mc *mc;
        int everr;
        int sslerr;

        everr = EVUTIL_SOCKET_ERROR();

        if (everr != 0)
        {
            log_warnx("[META] unexpected shutdown of the meta-connexion: (%d) %s",
                       everr, evutil_socket_error_to_string(everr));
        }
        while ((sslerr = bufferevent_get_openssl_error(bev)) != 0)
        {
            log_warnx("[META] SSL error code (%d): %s in %s %s",
                       sslerr, ERR_reason_error_string(sslerr),
                       ERR_lib_error_string(sslerr),
                       ERR_func_error_string(sslerr));
        }
        /*
         * Find if the exception come from a pending peer or a
         * regular peer and close it.
         */
        mc = v_mc_find_if(s->pending_peers, (void *)find_bev, bev);
        if (mc != v_mc_end(s->pending_peers))
        {
            char name[128];

            mc_close(mc);
            log_debug("[META] %s removed from the pending list",
                      mc_presentation(mc, name, sizeof name));
            v_mc_erase(s->pending_peers, mc);
        }
        else
        {
            mc = v_mc_find_if(s->peers, (void *)find_bev, bev);
            if (mc != v_mc_end(s->peers))
            {
                mc_close(mc);
                v_mc_erase(s->peers, mc);
                log_debug("[META] socket removed from the peer list");
            }
        }
    }
}

static void
listen_callback(struct evconnlistener *evl,
                evutil_socket_t fd,
                struct sockaddr *sock,
                int len,
                void *ctx)
{
    struct server *s = (struct server *)ctx;
    struct event_base *base = evconnlistener_get_base(evl);

    mc_peer_accept(s, base, sock, len, fd);
}


SSL_CTX *
evssl_init(void)
{
    SSL_CTX  *server_ctx;

    server_ctx = SSL_CTX_new(SSLv23_method());

    /* Load the certificate file. This is not needed now */
   /*! SSL_CTX_use_certificate_chain_file(server_ctx, "cert") ||*/
    if ((serv_opts.key_path != NULL && serv_opts.cert_path != NULL) &&
        (!SSL_CTX_use_certificate_chain_file(server_ctx, serv_opts.cert_path) ||
        !SSL_CTX_use_PrivateKey_file(server_ctx, serv_opts.key_path, SSL_FILETYPE_PEM)))
    {
        log_info("Couldn't read 'pkey' or 'cert' file.  To generate a key");
        log_info("and self-signed certificate, run:");
        log_info("  openssl genrsa -out pkey 2048");
        log_info("  openssl req -new -key pkey -out cert.req");
        log_info("  openssl x509 -req -days 365 -in cert.req -signkey pkey -out cert");
        return NULL;
    }
    return server_ctx;
}

static
void listen_client_callback(struct evconnlistener *evl, evutil_socket_t fd,
  struct sockaddr *sock, int len, void *ctx)
{
	struct server *s = (struct server *)ctx;
    struct event_base *base = evconnlistener_get_base(evl);
    int errcode;

	log_debug("A client is connecting");
    memset(&s->mc_client, 0, sizeof(s->mc_client));
    /* Notifiy the mc_init that we are in an SSL_ACCEPTING state*/
    /* Even if we are not in a SSL context, mc_init know what to do anyway*/
    s->mc_client.ssl_flags = BUFFEREVENT_SSL_ACCEPTING;
    errcode = mc_init(&s->mc_client, base, fd, sock, (socklen_t)len, s->server_ctx);
    if (errcode != -1)
    {
        bufferevent_setcb(s->mc_client.bev, client_mc_read_cb, NULL,
                          client_mc_event_cb, s);
        log_debug("[CLT] client connected");
    }
    else
    {
        log_notice("[CLT] Failed to init a meta connexion");
    }
}

int
server_init(struct server *s, struct event_base *evbase)
{
    struct cfg_sockaddress *it_listen = NULL;
    struct cfg_sockaddress *ite_listen = NULL;
    struct cfg_sockaddress *it_client = NULL;
    struct cfg_sockaddress *ite_client = NULL;
    struct cfg_sockaddress *it_peer = NULL;
    struct cfg_sockaddress *ite_peer = NULL;
    size_t i = 0;

    s->peers = v_mc_new();
    s->pending_peers = v_mc_new();
    s->srv_list = v_evl_new();
    s->frames_to_send = v_frame_new();
    s->evbase = evbase;

    it_listen = v_sockaddr_begin(serv_opts.listen_addrs);
    ite_listen = v_sockaddr_end(serv_opts.listen_addrs);
    /* Listen on all ListenAddress */
    for (; it_listen != ite_listen; it_listen = v_sockaddr_next(it_listen), ++i)
    {
        struct evconnlistener *evl = NULL;
        char listenname[INET6_ADDRSTRLEN];
        int err;

        evl = evconnlistener_new_bind(evbase, listen_callback,
            s, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
            (struct sockaddr *)&it_listen->sockaddr, it_listen->len);
        if (evl == NULL) {
            log_warnx("[INIT] failed to allocate the listener to listen to %s",
                address_presentation((struct sockaddr *)&it_listen->sockaddr,
                it_listen->len, listenname, sizeof listenname));
             continue;
        }
        evconnlistener_set_error_cb(evl, NULL);
        evconnlistener_disable(evl);

        /* udp endpoint init */

        /*
         * We listen on the same address for the udp socket
         */
        err = server_udp_init(s,
                              (struct sockaddr *)&it_listen->sockaddr,
                              it_listen->len);
        if (err == -1)
        {
            log_warnx("[INIT] [UDP] failed to init the udp socket on %s",
                  address_presentation((struct sockaddr *)&it_listen->sockaddr,
                                           it_listen->len, listenname,
                                           sizeof listenname));
            continue;
        }

        v_evl_push(s->srv_list, evl);
    }

	// Listen enable for client in the ports registered
    it_client = v_sockaddr_begin(serv_opts.client_addrs);
    ite_client = v_sockaddr_end(serv_opts.client_addrs);
    /* Listen on all ClientAddress */
    for (; it_client != ite_client; it_client = v_sockaddr_next(it_client), ++i)
    {
        char clientname[INET6_ADDRSTRLEN];
        struct evconnlistener *evl = NULL;
		evl = evconnlistener_new_bind(evbase, listen_client_callback,
			  s, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
		  (struct sockaddr *)&it_client->sockaddr, it_client->len);
		if (evl == NULL) {
			log_warnx("[INIT] [TCP] failed to allocate the listener to listen to %s",
			  address_presentation((struct sockaddr *)
				&it_client->sockaddr, it_client->len, clientname,
				sizeof clientname));
			continue;
		}
		evconnlistener_set_error_cb(evl, NULL);
		evconnlistener_enable(evl);
		v_evl_push(s->srv_list, evl);
	}

    /* If we don't have any PeerAddress it's finished */
    if (v_sockaddr_size(serv_opts.peer_addrs) == 0)
        return 0;

    it_peer = v_sockaddr_begin(serv_opts.peer_addrs);
    ite_peer = v_sockaddr_end(serv_opts.peer_addrs);
    for (;it_peer != ite_peer; it_peer = v_sockaddr_next(it_peer))
    {
        mc_peer_connect(s, evbase,
                        (struct sockaddr *)&it_peer->sockaddr,
                        it_peer->len);
    }
    return 0;
}

void server_delete(struct server *s)
{
    /* Start by the servers */
    v_evl_foreach(s->srv_list, evconnlistener_free);
    server_udp_exit(&s->udp);

    /* Then the tuntap device */
    event_free(s->device);

    /* Clean the vectors */
    v_mc_foreach(s->pending_peers, (void (*)(struct mc const *))mc_close);
    v_mc_foreach(s->peers, (void (*)(struct mc const *))mc_close);
    v_frame_foreach(s->frames_to_send, frame_free);

    /* Free the actual vector memory */
    v_mc_delete(s->pending_peers);
    v_mc_delete(s->peers);
    v_frame_delete(s->frames_to_send);
    v_evl_delete(s->srv_list);

    /* Free the SSL_CTX if we allocated it */
    if (s->server_ctx != NULL)
        SSL_CTX_free(s->server_ctx);

    /* We need a way to know if there is a client or not */
    /*mc_close(&s->mc_client);*/
#if defined Windows
    bufferevent_free(s->pipe_endpoint);
#endif
}
