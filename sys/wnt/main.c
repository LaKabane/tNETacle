/*
 * Copyright (c) 2012 Raphael "Arty" Thoulouze <raphael.thoulouze@gmail.com>
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

#include <event2/event.h>

#include "tnetacle.h"
#include "tntexits.h"
#include "log.h"
#include "tun.h"
#include "server.h"

int
main(int argc, char *argv[]) {

	WSADATA wsaData;
	int errcode;
    struct event_base *evbase = NULL;
    struct server server;

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
	log_err(-1, "Failed to init WSA.");
	}
	if ((evbase = event_base_new()) == NULL) {
	log_err(-1, "Failed to init the event library");
    }

    //sigterm = event_new(evbase, SIGTERM, EV_SIGNAL, &chld_sighdlr, evbase);
    //sigint = event_new(evbase, SIGINT, EV_SIGNAL, &chld_sighdlr, evbase);

    if (server_init(&server, evbase) == -1)
	log_err(-1, "Failed to init the server socket");


    //event_add(sigterm, NULL);
    //event_add(sigint, NULL);

    log_info("tnetacle ready");


    log_info("Starting event loop");
	if (event_base_dispatch(evbase) == -1) {
		errcode = WSAGetLastError();
	}


    /*
     * It may look like we freed this one twice, once here and once in tnetacled.c
     * but this is not the case. Please don't erase this !
     */
    event_base_free(evbase);

    //event_free(sigterm);
    //event_free(sigint);

    log_info("tnetacle exiting");
    return TNT_OK;
}