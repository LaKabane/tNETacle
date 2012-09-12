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

#include "client.h"
#include "tclt_json.h"

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
			log_debug(" %d   [%s]\n", size, buff);
			ele = tclt_parse(buff, size);
			/* while (ele != NULL) */
			/* { */
			/* 	log_debug("%d\n", ele->type); */
			/* 	ele = ele->next; */
			/* } */
		}
	}
}
