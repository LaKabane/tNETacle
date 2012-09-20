/*
 * Copyright (c) 2012 Florent Tribouilloy <tribou_f AT epitech DOT net>
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


#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined Unix
# include <unistd.h>
#endif

#include <event2/bufferevent.h>

#include "client.h"
#include "tclt_json.h"
#include "server.h"

static int
_server_match_bev(struct mc const *a, struct mc const *b)
{
    return a->bev == b->bev;
}

void
client_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = (struct server *)ctx;
    size_t size;
    struct evbuffer *buf = NULL;
    char buff[4096];
    elements *ele;
    int i;

    buf = bufferevent_get_input(bev);
    while (evbuffer_get_length(buf) != 0)
    {
        size = bufferevent_read(bev, buff, 4095);
	if (size != 0 && size != -1)
        {
            buff[size] = '\0';
            log_debug("%s\n", buff);
            ele = tclt_parse(buff, size);
            while (ele != NULL)
            {
                if (ele->type == 7)
                    log_debug("%s\n", ele->u_value.buf);
                ele = ele->next;
            }
        }
    }
}

void
client_mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    struct server *s = (struct server *)ctx;

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
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            log_info("connexion established.");
            memcpy(&tmp, mc, sizeof(tmp));
            v_mc_erase(&s->pending_peers, mc);
            v_mc_push(&s->peers, &tmp);
        }
		//send peers to the client
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
        mc = v_mc_find_if(&s->pending_peers, &tmp, _server_match_bev);
        if (mc != v_mc_end(&s->pending_peers))
        {
            mc_close(mc);
            v_mc_erase(&s->pending_peers, mc);
            log_debug("socket removed from the pending list");
        }
        else
        {
            mc = v_mc_find_if(&s->peers, &tmp, _server_match_bev);
            if (mc != v_mc_end(&s->peers))
            {
                mc_close(mc);
                v_mc_erase(&s->peers, mc);
                log_debug("socket removed from the peer list");
            }
        }
    }
}
