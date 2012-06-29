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
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

#include <openssl/err.h>

#include "mc.h"
#include "log.h"
int
mc_init(struct mc *self, struct event_base *evb, int fd, struct sockaddr *s,
        socklen_t len, SSL_CTX *server_ctx)
{
    struct sockaddr *tmp = malloc(len);

    if (server_ctx != NULL)
    {
        self->client_ctx = SSL_new(server_ctx);
        self->bev = bufferevent_openssl_socket_new(evb, fd, self->client_ctx,
                                                   self->ssl_flags,
                                                   BEV_OPT_CLOSE_ON_FREE);
    }
    else
    {
        self->client_ctx = NULL;
        self->bev = bufferevent_socket_new(evb, fd, BEV_OPT_CLOSE_ON_FREE);
    }

    
    if (tmp == NULL || self->bev == NULL)
    {
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

int
mc_ssl_connect(struct mc *self, struct event_base *evbase)
{
    int err;
    struct bufferevent *ssl_bev = 
        bufferevent_openssl_filter_new(evbase, self->bev,
                                       self->client_ctx,
                                       BUFFEREVENT_SSL_CONNECTING,
                                       BEV_OPT_CLOSE_ON_FREE);
    if (ssl_bev == NULL)
    {
        return -1;
    }
    err = SSL_connect(self->client_ctx);
    switch (err)
    {
        case 1:
            log_notice("SSL handshake sucessful, secure connexion initited.");
            return 0;
            break;
        case 2:
            log_notice("SSL handshake not sucessful");
            log_notice("SSL Status: %s.", 
                       ERR_reason_error_string(SSL_get_error(self->client_ctx, err)));
            break;
        default:
            log_notice("Fatal error during SSL handshake");
            log_notice("SSL Status: %s.", 
                       ERR_reason_error_string(SSL_get_error(self->client_ctx, err)));
            break;
    };
    return -1;
}

int
mc_ssl_accept(struct mc *self, struct event_base *evbase)
{
    int err;
    char buf[4096] = {0};
    struct bufferevent *ssl_bev = 
        bufferevent_openssl_filter_new(evbase, self->bev,
                                       self->client_ctx,
                                       BUFFEREVENT_SSL_ACCEPTING,
                                       BEV_OPT_CLOSE_ON_FREE);
    if (ssl_bev == NULL)
    {
        return -1;
    }
    err =  SSL_accept(self->client_ctx);
    switch (err)
    {
        case 1:
            log_notice("SSL handshake sucessful, secure connexion initited.");
            return 0;
            break;
        case 2:
            log_notice("SSL handshake not sucessful");
            log_notice("SSL Status: %s.", 
                       ERR_error_string(SSL_get_error(self->client_ctx, err), buf));
            break;
        default:
            log_notice("Fatal error during SSL handshake");
            log_notice("SSL Status: %s.", 
                       ERR_error_string(SSL_get_error(self->client_ctx, err), buf));
            break;
    };
    return -1;
}

void
mc_close(struct mc *self)
{
    free(self->p.address);
    bufferevent_free(self->bev);
}
