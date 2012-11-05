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

#include <stdarg.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/event-config.h>
#include <event2/bufferevent_ssl.h>

#include <openssl/err.h>

#include "log.h"
#include "client.h"
#include "tclt.h"
#include "tclt_parse.h"
#include "tclt_command.h"
#include "server.h"
#include "options.h"

void
client_mc_read_cb(struct bufferevent *bev, void *ctx)
{
    struct server *s = (struct server *)ctx;
    size_t size;
    struct evbuffer *buf = NULL;
    char buff[4096];
    /*elements *ele;*/

    buf = bufferevent_get_input(bev);
    while (evbuffer_get_length(buf) != 0)
    {
        size = bufferevent_read(bev, buff, 4095);
        if (size != 0)
        {
            buff[size] = '\0';
            tclt_dispatch_command(buff);
        }
    }
}

void
client_mc_event_cb(struct bufferevent *bev, short events, void *ctx)
{
    (void)ctx;
    if (events & BEV_EVENT_ERROR)
    {
        int everr;
        int sslerr;

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
    }
    else if (events & BEV_EVENT_EOF)
    {
        //BEV_EVENT_ERROR_EOF == end of connection
		log_warnx("Client shutdown... WOOOOOOTTTT!!!");
    }
}

static int
add_peer(void *f)
{
    peer *p = (peer*)f;

    printf("add a new peer : %s  %s %s\n", p->name, p->ip, p->key);
    return 0;
}

void
client_init_callback(void)
{
    tclt_set_callback_command(ADD_PEER_CMD, add_peer);
}
