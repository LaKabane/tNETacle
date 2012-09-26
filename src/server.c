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

extern struct options serv_opts;

void
server_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = (struct server *)ctx;
    struct evbuffer *in = bufferevent_get_input(bev);

    /* Get rid of every input */
    (void)s;
    evbuffer_drain(in, evbuffer_get_length(in));
}

static int
_server_match_bev(struct mc const *a, struct mc const *b)
{
    return a->bev == b->bev;
}

void
server_mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    struct server *s = (struct server *)ctx;

    (void)bev;
    (void)s;
    if (events & BEV_EVENT_CONNECTED)
    {
        struct mc *mc;
        struct mc tmp;

        /*
         * If we received the notification that the connection is established,
         * then we move the corresponding struct mc from s->pending_peers to
         * s->peers.
         */

        tmp.bev = bev;
        mc = v_mc_find_if(s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(s->pending_peers))
        {
            log_info("connexion established.");
            memcpy(&tmp, mc, sizeof(tmp));
            v_mc_erase(s->pending_peers, mc);
            v_mc_push(s->peers, &tmp);
            mc_hello(&tmp);
        }
    }
    if (events & BEV_EVENT_ERROR)
    {
        struct mc *mc;
        struct mc tmp;
        int everr;
        int sslerr;

        tmp.bev = bev;
        everr = EVUTIL_SOCKET_ERROR();

        if (everr != 0)
        {
            log_warnx("Unexpected shutdown of the meta-connexion: (%d) %s",
                       everr, evutil_socket_error_to_string(everr));
        }
        while ((sslerr = bufferevent_get_openssl_error(bev)) != 0)
        {
            log_warnx("SSL error code (%d): %s in %s %s",
                       sslerr, ERR_reason_error_string(sslerr),
                       ERR_lib_error_string(sslerr),
                       ERR_func_error_string(sslerr));
        }
        /*
         * Find if the exception come from a pending peer or a
         * regular peer and close it.
         */
        mc = v_mc_find_if(s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(s->pending_peers))
        {
            char name[128];

            mc_close(mc);
            log_debug("%s removed from the pending list",
                      mc_presentation(mc, name, sizeof name));
            v_mc_erase(s->pending_peers, mc);
        }
        else
        {
            mc = v_mc_find_if(s->peers, &tmp, _server_match_bev);
            if (mc != v_mc_end(s->peers))
            {
                mc_close(mc);
                v_mc_erase(s->peers, mc);
                log_debug("socket removed from the peer list");
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

    /* Initialize the OpenSSL library */
    SSL_load_error_strings();
    SSL_library_init();
    /* We MUST have entropy, or else there's no point to crypto. */
    if (!RAND_poll())
        return NULL;

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
    struct mc mc;

	log_debug("A client is connecting");
    memset(&mc, 0, sizeof mc);
    /* Notifiy the mc_init that we are in an SSL_ACCEPTING state*/
    /* Even if we are not in a SSL context, mc_init know what to do anyway*/
    mc.ssl_flags = BUFFEREVENT_SSL_ACCEPTING;
    errcode = mc_init(&mc, base, fd, sock, (socklen_t)len, s->server_ctx);
    if (errcode != -1)
    {
        bufferevent_setcb(mc.bev, client_mc_read_cb, NULL,
                          client_mc_event_cb, s);
        log_debug("client connected");
        //v_mc_push(&s->peers, &mc);
    }
    else
    {
        log_notice("Failed to init a meta connexion");
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
            log_warnx("Failed to allocate the listener to listen to %s",
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
        err = server_init_udp(s,
                              (struct sockaddr *)&it_listen->sockaddr,
                              it_listen->len);
        if (err == -1)
        {
            log_warnx("Failed to init the udp socket on %s",
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
			log_warnx("Failed to allocate the listener to listen to %s",
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
        mc_peer_connect(s, evbase, (struct sockaddr *)&it_peer->sockaddr,
                        it_peer->len);
    }
    return 0;
}
