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

#if defined Unix
# include <unistd.h>
#endif

#include <event2/event.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "networking.h"
#include "tntsched.h"
#include "endpoint.h"
#include "options.h"
#include "dtls.h"
#include "udp.h"
#include "log.h"

extern struct options serv_opts;

void
log_ssl(char const *msg, ...)
{
#if !defined(Windows) /* TODO */
	va_list ap;
    char *fmt;
    char *log;
    char errstr[150];
    unsigned long errval;

    va_start(ap, msg);
    errval = ERR_get_error();
    ERR_error_string_n(errval, errstr, sizeof(errstr));
    asprintf(&fmt, "%s (%s)", msg, errstr);
    vasprintf(&log, fmt, ap);
    log_warnx(log);
    free(fmt);
    free(log);
    va_end(ap);
#endif
}

SSL_CTX *
create_udp_ctx(void)
{
    int err;
    char const *key_path;
    char const *certfile_path;

    SSL_CTX *ctx = SSL_CTX_new(DTLSv1_method());
    if (ctx == NULL)
    {
        log_ssl("[DTLS] unable to create the dtls context");
        return NULL;
    }

    err = SSL_CTX_set_cipher_list(ctx, "HIGH:MEDIUM:aNULL");
    if (err != 1)
    {
        log_ssl("[DTLS] Unable to load ciphers, exiting");
        return NULL;
    }

    key_path = serv_opts.key_path;
    if (key_path == NULL)
    {
        log_warnx("[DTLS] Unable to load private key file");
        return NULL;
    }

    err = SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM);
    if (err == -1)
    {
        log_ssl("[DTLS] unable to load the private key file");
        return NULL;
    }

    certfile_path = serv_opts.cert_path;
    if (certfile_path == NULL)
    {
        log_warnx("[DTLS] Unable to load certificate file");
        return NULL;
    }

    err = SSL_CTX_use_certificate_chain_file(ctx, certfile_path);
    if (err == -1)
    {
        log_ssl("[DTLS] unable to load the certificate chain file");
        return NULL;
    }
    return ctx;
}

int
dtls_new_peer(SSL_CTX *ctx, struct udp_peer *p)
{
    int err;
    SSL *ssl;

    err = BIO_new_bio_pair(&p->bio, TNETACLE_UDP_MTU, &p->_bio_backend, TNETACLE_UDP_MTU);
    if (err != 1)
    {
        log_ssl("[DTLS] unable to create the Buffered IO needed for dtls multiplexing");
        return -1;
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        log_ssl("[DTLS] unable to instanciate SSL for the current peer");
        return -1;
    }

    SSL_set_bio(ssl, p->bio, p->bio);

    ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        BIO_free(p->bio);
        BIO_free(p->_bio_backend);
        return -1;
    }
    p->ssl = ssl;

    if (p->ssl_flags & DTLS_CLIENT)
    {
        SSL_set_connect_state(ssl);
    }
    else
    {
        SSL_set_accept_state(ssl);
    }
    return 0;
}

ssize_t
dtls_do_handshake(int fd,
                  struct udp *udp,
                  struct udp_peer *peer,
                  void *async_ctx)
{
    (void)fd;
    (void)udp;
    (void)peer;
    (void)async_ctx;
    return -1;
}

ssize_t
dtls_sendto(int sockfd,
            void const *buf,
            size_t len,
            int flags,
            struct sockaddr const *addr,
            int socklen,
            struct udp_peer *peer,
            void *async_ctx)
{
    int err;

    err = SSL_write(peer->ssl, buf, len);
    if (err != (int)len)
    {
        /*SSL_write return less bytes written than expected */
        log_ssl("[DTLS] failed to write on the ssl endpoint");
    }
    else
    {
        unsigned char tbuf[TNETACLE_UDP_MTU];
        int ret;

        ret = BIO_read(peer->bio, tbuf, sizeof(tbuf));
        if (ret == 0)
        {
            /* Nothing  was read, and it is normal to quit */
            return 0;
        }
        else if (ret < 0)
        {
            /* Something wrong happend */
            /*XXX: check the error code*/
            log_ssl("[DTLS] dtls_sendto: BIO_read error");
        }

        err = async_sendto(async_ctx,
                           sockfd,
                           tbuf,
                           ret,
                           flags,
                           addr,
                           socklen);
        /* If we didn't send all the bytes: */
        if (err != ret)
        {
            log_warn("[DTLS] sendto");
            return -1;
        }
        return err;
    }
    return -1;
}

static int
find_peer(struct udp_peer const *P, void *ctx) 
{
    struct sockaddr *addr = ctx;
    return !evutil_sockaddr_cmp((struct sockaddr *)&P->peer_addr.addr, addr, 1);
}

/* Not functional */
ssize_t
dtls_recvfrom(int sockfd,
              void *buf,
              size_t len,
              int flags,
              struct sockaddr *addr,
              int *socklen,
              struct udp *udp,
              void *async_ctx)
{
    int nread;
    unsigned char tbuf[TNETACLE_UDP_MTU];

    nread = async_recvfrom(async_ctx,
                         sockfd,
                         (char *)tbuf,
                         sizeof(tbuf),
                         flags,
                         addr,
                         socklen);
    if (nread == -1)
    {
        log_warn("[DTLS] async_recvfrom");
        return -1;
    }
    else
    {
        int err;
        struct udp_peer *peer;
        int pending;

        peer = v_udp_find_if(udp->udp_peers, find_peer, (void *)addr);
        if (peer == NULL)
        {
            struct endpoint e;

            endpoint_init(&e, addr, *socklen);
            peer = udp_register_new_peer(udp,
                                         &e,
                                         DTLS_ENABLE | DTLS_SERVER);
        }
        BIO_write(peer->bio, tbuf, nread);
        err = SSL_read(peer->ssl, buf, len);
        if (err < 0)
        {
            log_ssl("[DTLS] unable to read on the ssl endpoint");
        }
        if ((pending = BIO_ctrl_pending(peer->bio)) > 0)
        /* Some data are pending */
        {
            int size;

            size = BIO_read(peer->bio, tbuf, TNETACLE_UDP_MTU);
            async_sendto(async_ctx,
                         sockfd,
                         tbuf,
                         size,
                         flags,
                         addr,
                         *socklen);

        }
        return err;
    }
    return -1;
}
