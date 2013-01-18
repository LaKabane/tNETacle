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

#include "networking.h"

#include <openssl/sha.h>

#include "tnetacle.h"
#include "mc.h"
#include "tntsocket.h"
#include "server.h"
#include "log.h"
#include "wincompat.h"
#include "udp.h"
#include "frame.h"
#include "device.h"
#include "tntsched.h"
#include "subset.h"

#include <dtls.h>

char const *
udp_session_presentation(session_t *session)
{
    static struct endpoint e;

    endpoint_init(&e, &session->addr.sa, session->size);
    return endpoint_presentation(&e);
}

int
udp_connect(struct udp *udp, struct endpoint *e)
{
    session_t session;

    memset(&session, '\0', sizeof(session));
    session.size = endpoint_addrlen(e);
    memcpy(&session.addr, endpoint_addr(e), session.size);
    return dtls_connect(udp->ctx, &session);
}

int
udp_broadcast(struct udp *udp, char const *data, size_t len)
{
    int gerr = 0;
    dtls_peer_t *p = NULL;
    dtls_peer_t *tmp;

    if (udp == NULL || udp->ctx == NULL || udp->ctx->peers == NULL)
        return -1;
    /* For all the peers*/

    HASH_ITER(hh, udp->ctx->peers, p, tmp)
    {
        int err;
        struct packet_hdr hdr;

        if (p->state != DTLS_STATE_CONNECTED)
            continue;
        err = dtls_write(udp->ctx, &p->session, data, len);
        if (err == -1)
        {
            gerr = -1;
            log_warn("[UDP] [DTLS] error while sending to %s",
                    udp_session_presentation(&p->session));
        }
        log_debug("[UDP] [DTLS] sending %d(%-#2x) bytes to %s",
                err, err,
                udp_session_presentation(&p->session));
    }
    return gerr;
}

static int
send_to_peer(struct dtls_context_t *ctx,
             session_t *session,
             uint8 *data,
             size_t len)
{
    struct server *s = (struct server *)dtls_get_app_data(ctx);
    int err;

    err = sendto(s->udp->fd,
                 data,
                 len,
                 MSG_DONTWAIT,
                 &session->addr.sa,
                 session->size);
    if (err < 0)
    {
        log_warn("[UDP] sendto error, code %i", err);
    }
    else
    {
        log_debug("[UDP] sending %d(%-#2x) bytes to %s",
                err, err,
                udp_session_presentation(session));
    }
    return err;
}

static int
read_from_peer(struct dtls_context_t *ctx,
               session_t *session,
               uint8 *data,
               size_t len)
{
    struct server *s = (struct server *)dtls_get_app_data(ctx);
#if defined Windows
    struct frame current_frame;

    current_frame.frame = data;
    current_frame.size = len;
    /*
     * Send to current frame to the windows thread handling the tun/tap
     * devices and clean the evbuffer
     */
    send_buffer_to_device_thread(s, &current_frame);
#else
    /* Write the current frame on the device */
    write(s->tap_fd, data, len);
#endif
    /* this is ignored by tinydtls */
    return 0;
}

static dtls_key_t dtls_key = {
    .type = DTLS_KEY_PSK,
    .key.psk.id = (unsigned char *)"Client_identity", 
    .key.psk.id_length = 15,
    .key.psk.key = (unsigned char *)"secretPSK", 
    .key.psk.key_length = 9
};

static int
get_key(struct dtls_context_t *ctx,
        session_t const *session,
        unsigned char const *id,
        size_t id_len,
        dtls_key_t const ** result)
{
    *result = &dtls_key;
    log_debug("[UDP] [DTLS] getting the key for %s", id);
    return 0;
}

static int
udp_event(struct dtls_context_t *ctx,
          session_t *session,
          dtls_alert_level_t level,
          unsigned short code)
{
    log_debug("[UDP] [DTLS] Got event alert level: %i code: %i", level, code);
    return 0;
}

static dtls_handler_t dtls_handler = {
    .write = send_to_peer,
    .read = read_from_peer,
    .event = udp_event,
    .get_key = get_key
};

static void
udp_handle_read(void *ctx)
{
    struct server *s = (struct server *)sched_get_userptr(ctx);
    evutil_socket_t udp_fd;
    struct endpoint e;
    session_t session;
    static char data[DTLS_MAX_BUF];
    int err;

    udp_fd = s->udp->fd;
    memset(&session, '\0', sizeof(session));
    session.size = sizeof(session.addr);

    do
    {
        err = async_recvfrom(ctx,
                             udp_fd,
                             data,
                             sizeof(data),
                             0,
                             (struct sockaddr *)&session.addr.sa,
                             (int *) &session.size);
        /*
        ** Sometimes, on some OS, recvfrom return EMSGSIZE when the size of the
        ** peeked buffer is not enough to read the entire datagram.
        ** So we need to check for this specific case.
        */
        if (err == -1)
        {
            int local_err = EVUTIL_SOCKET_ERROR();

#if defined Windows
#define MSG_TOO_LONG WSAEMSGSIZE
#else
#define MSG_TOO_LONG EMSGSIZE
#endif
            if (local_err != MSG_TOO_LONG)
            {
                /* So there is a real error */
                break;
            }
#undef MSG_TOO_LONG
        }

        if (err > 0)
        {
            int len = err;

            log_debug("[UDP] recving %d(%-#2x) from %s",
                      len,
                      len,
                      udp_session_presentation(&session));

            err = dtls_handle_message(s->udp->ctx, &session, (uint8 *)data, len);
            if (err < 0)
            {
                log_debug("[UDP] [DTLS] an error occured, code: %i", err);
            }
        }
        else
        {
            log_debug("[UDP] empty datagram??");
        }
    }
    while (1);
    sched_fiber_exit(ctx, 1);
}

void
udp_exit(struct udp *udp)
{
    if (udp == NULL)
        return;
    (void)close((int)udp->fd);
    dtls_free_context(udp->ctx);
    sched_fiber_delete(udp->fib);
}

int
udp_bind(struct udp *udp,
         struct endpoint *e)
{
    int err;

    err = bind(udp->fd,
               endpoint_addr(e),
               e->addrlen);
    if (err == -1)
    {
        log_warn("[INIT] [UDP] binding: ");
        return -1;
    }
    log_debug("[UDP] socket listening on %s",
              endpoint_presentation(e));
    return 0;
}

int
udp_init(struct server *s,
         struct udp *udp)
{
    int             err;
    evutil_socket_t tmp_sock = 0;

    tmp_sock = tnt_udp_socket(AF_INET);
    if (tmp_sock == -1)
    {
        log_warn("[UDP] [INIT] socket creation failed:");
        return -1;
    }
    err = evutil_make_socket_nonblocking(tmp_sock);
    if (err == -1)
    {
        return -1;
    }
    udp->fd = tmp_sock;
    udp->fib = sched_new_fiber(s->ev_sched, udp_handle_read,
            (intptr_t)s);
    udp->ctx = dtls_new_context(s);

    dtls_set_handler(udp->ctx, &dtls_handler);
    return 0;
}

void
udp_launch(struct udp *u)
{
    log_debug("[UDP] Launching fiber");
    if (u != NULL && u->fib != NULL)
        sched_fiber_launch(u->fib);
}

struct udp *
udp_new(struct server *s)
{
    int err;
    struct udp *udp;

    udp = tnt_new(struct udp);
    if (udp == NULL)
    {
        return NULL;
    }
    err = udp_init(s, udp);
    if (err == -1)
    {
        free(udp);
        return NULL;
    }
    return udp;
}

unsigned short
udp_get_port(struct udp *udp)
{
    return endpoint_port(&udp->udp_endpoint);
}
