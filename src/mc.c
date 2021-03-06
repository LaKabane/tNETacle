/**
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**/

#include <stdlib.h>
#include <string.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/util.h>

#include <openssl/err.h>

#include "networking.h"
#include "mc.h"
#include "log.h"
#include "server.h"
#include "tnetacle.h"
#include "options.h"
#include "udp.h"

extern struct options serv_opts;

/*
 * A simple wrapper over inet_ntop :)
 */

char *
address_presentation(struct sockaddr *sock,
                     int socklen,
                     char *name,
                     int namelen)
{
    (void)socklen;
    if (sock->sa_family == AF_INET)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)sock;
        char tmp[INET_ADDRSTRLEN];

        evutil_inet_ntop(AF_INET, &sin->sin_addr, tmp, sizeof tmp);
        evutil_snprintf(name, namelen, "%s:%d", tmp, ntohs(sin->sin_port));
        return name;
    }
    else
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sock;
        char tmp[INET6_ADDRSTRLEN];

        evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, tmp, sizeof tmp);
        evutil_snprintf(name, namelen, "%s:%d", tmp, ntohs(sin6->sin6_port));
        return name;
    }
}

/*
 * Returns a human readable presentation of the mc endpoint.
 * Fills the char *name parameter, and return its address.
 */
char *
mc_presentation(struct mc *self,
                char *name,
                int len)
{
    struct sockaddr *sock = self->p.address;
    int socklen = self->p.len;

    return address_presentation(sock, socklen, name, len);
}

/*
 * This function init a new struct mc.
 * struct event_base *evb: a pointer to a correct event_base
 * int fd: optional, an fd already opened to create the underlying event layer.
 *     -1 for creating the socket here.
 * struct sockaddr *s, socklent_t len: pointer to the coresponding sockaddr
 *     and its len. It will be copied inside.
 * SSL_CTX* server_ctx: optional, the ssl context of the server.
 *     Used to initialised the underlying bufferevent_ssl. Can be NULL.
 *
 */

int
mc_init(struct mc *self,
        struct event_base *evb,
        evutil_socket_t fd,
        struct sockaddr *s,
        socklen_t len,
        SSL_CTX *server_ctx)
{
    struct sockaddr *tmp = malloc(len);

    if (server_ctx != NULL)
    {
        SSL *client_ctx = SSL_new(server_ctx);
        self->bev = bufferevent_openssl_socket_new(evb, fd, client_ctx,
                                                   self->ssl_flags,
                                                   BEV_OPT_CLOSE_ON_FREE);
        self->ssl_flags = TLS_ENABLE;
    }
    else
    {
        self->ssl_flags = TLS_DISABLE;
        self->bev = bufferevent_socket_new(evb, fd, BEV_OPT_CLOSE_ON_FREE);
    }


    if (tmp == NULL || self->bev == NULL)
    {
        bufferevent_free(self->bev);
        free(tmp);
        log_notice("failed to allocate the memory needed to establish a new "
                   "meta-connection");
        return -1;
    }
    memcpy(tmp, s, len);
    self->p.address = tmp;
    self->p.len = len;
    bufferevent_enable(self->bev, EV_READ | EV_WRITE);
    return 0;
}

struct mc *
mc_peer_connect(struct server *s,
                struct event_base *evbase,
                struct sockaddr *sock,
                int socklen)
{
    int err;
    struct mc tmp;
    char peername[INET6_ADDRSTRLEN];

    address_presentation(sock, socklen, peername, sizeof(peername));
    memset(&tmp, 0, sizeof(tmp));
    tmp.ssl_flags = BUFFEREVENT_SSL_CONNECTING;
    if (mc_established(s, sock, socklen) != 0)
    {
        log_notice("[META] [CONNECT] connexion already established with %s", peername);
        return NULL;
    }
    err = mc_init(&tmp, evbase, -1, sock, socklen, s->server_ctx);
    if (err == -1) {
        log_warn("[META] [CONNECT] unable to allocate a socket for connecting to %s",
                 peername);
        return NULL;
    }
    bufferevent_setcb(tmp.bev, server_mc_read_cb, NULL, server_mc_event_cb, s);
    err = bufferevent_socket_connect(tmp.bev, sock, socklen);
    if (err == -1) {
        log_warn("[META] unable to connect to %s", peername);
        return NULL;
    }
    bufferevent_disable(tmp.bev, EV_READ|EV_WRITE);
    return v_mc_insert(s->pending_peers, &tmp);
}

struct mc *
mc_peer_accept(struct server *s,
               struct event_base *evbase,
               struct sockaddr *sock,
               int socklen,
               evutil_socket_t fd)
{
    int errcode;
    struct mc mc;
    char peername[INET6_ADDRSTRLEN];

    memset(&mc, 0, sizeof mc);
    address_presentation(sock, socklen, peername, sizeof(peername));
    /* Notifiy the mc_init that we are in an SSL_ACCEPTING state*/
    /* Even if we are not in a SSL context, mc_init know what to do anyway*/
    mc.ssl_flags = BUFFEREVENT_SSL_ACCEPTING;
    if (mc_established(s, sock, socklen) != 0)
    {
        log_notice("[META] [ACCEPT] connexion already established with %s", peername);
        /* TODO: write a tnet_close_socket(intptr_t) */
        close((int)fd);
        return NULL;
    }
    errcode = mc_init(&mc, evbase, fd, sock, socklen, s->server_ctx);
    if (errcode == -1)
    {
        log_notice("[META] [ACCEPT] failed to open a meta connexion with %s", peername);
        return NULL;
    }
    bufferevent_setcb(mc.bev,
                      server_mc_read_cb,
                      NULL,
                      server_mc_event_cb,
                      s);
    if (mc.ssl_flags & TLS_ENABLE)
    {
        /* the handshake hasn't been done yet */
        log_debug("[META] [TLS] waiting for the ssl handshake with %s", peername);
        return v_mc_insert(s->pending_peers, &mc);
    }
    log_debug("[META] opening a meta-connexion with %s", peername);
    /* XXX HACK HACK HACK XXX */
    bufferevent_disable(mc.bev, EV_READ|EV_WRITE);
    return v_mc_insert(s->pending_peers, &mc);
}


void
mc_close(struct mc *self)
{
    if (self->ssl_flags & TLS_ENABLE)
    {
        SSL *ssl = bufferevent_openssl_get_ssl(self->bev);

        SSL_set_shutdown(ssl, SSL_RECEIVED_SHUTDOWN);
        SSL_shutdown(ssl);
    }
    free(self->p.address);
    bufferevent_free(self->bev);
}

/*
 * This function will add raw data to the output buffer.
 * As you might guess, this function can not check anything about the input
 * data, so try not to use it to much.  The return value is -1 if the function
 * failed, the number of bytes added otherwise
 */

/* This one is useless */

size_t
mc_add_raw_data(struct mc *self,
                void *data,
                size_t size)
{
    struct evbuffer *output = bufferevent_get_output(self->bev);
    char name[128];
    int err;

    err = evbuffer_add(output, data, size);
    if (err == -1)
    {
        log_notice("error while add %d raw data into %p's output buffer",
            size, mc_presentation(self, name, sizeof name));
    }
    return size;
}

/*
 * This function is called when we connect to a peer.
 * tNETacle is a polite software, using a polite protocol, so we say hello !
 */
int
mc_hello(struct mc *self, struct udp *udp)
{
    struct evbuffer *output = bufferevent_get_output(self->bev);

    (void)udp;
    bufferevent_enable(self->bev, EV_READ|EV_WRITE);
    evbuffer_add_printf(output, "Hello ~!\r\n");
    return 0;
}

int
mc_establish_tunnel(struct mc *self, struct udp *udp)
{
    struct evbuffer *output = bufferevent_get_output(self->bev);
    unsigned short port = udp_get_port(udp);

    evbuffer_add_printf(output, "udp_port:%d\r\n", port);
    return 0;
}

static int
find_established(struct mc const *mc, void *ctx)
{
    struct sockaddr *test = (struct sockaddr *)ctx;
   
    return evutil_sockaddr_cmp(mc->p.address, test, 0) == 0;
}

int
mc_established(struct server *s, struct sockaddr *sck, int socklen)
{
    struct mc * it;

    (void)socklen;
    it = v_mc_find_if(s->peers, find_established, sck);
    return !(it == v_mc_end(s->peers));
}

int
mc_pending(struct server *s, struct sockaddr *sck, int socklen)
{
    struct mc * it;

    (void)socklen;
    it = v_mc_find_if(s->pending_peers, find_established, sck);
    return !(it == v_mc_end(s->pending_peers));
}


