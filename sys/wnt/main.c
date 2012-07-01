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

#include <WS2tcpip.h>

#include <event2/event.h>

#include "tnetacle.h"
#include "tntexits.h"
#include "log.h"
#include "tun.h"
#include "options.h"
#include "server.h"
#include "pathnames.h"

int debug;
extern struct options serv_opts;

static void
libevent_dump(struct event_base *base)
{
	int i;
	enum event_method_feature f;
	const char **methods = event_get_supported_methods();

	printf("Starting Libevent %s.  Available methods are:\n",
		event_get_version());
	for (i=0; methods[i] != NULL; ++i) {
		printf("    %s\n", methods[i]);
	}

    printf("Using Libevent with backend method %s.",
        event_base_get_method(base));
	f = event_base_get_features(base);
    if ((f & EV_FEATURE_ET))
        printf("  Edge-triggered events are supported.");
    if ((f & EV_FEATURE_O1))
        printf("  O(1) event notification is supported.");
    if ((f & EV_FEATURE_FDS))
        printf("  All FD types are supported.");
    puts("");
}

void init_options(struct options *);

int
main(int argc, char *argv[]) {

	WSADATA wsaData;
	int errcode;
    struct event_base *evbase = NULL;
    struct server server;
	struct device *interfce;
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof sin);
	init_options(&serv_opts);
	serv_opts.listen_addrs_num = 1;
	serv_opts.addr_family = AF_INET;
	serv_opts.listen_addrs = calloc(1, sizeof(struct sockaddr));

	sin.sin_family = AF_INET;
    sin.sin_port = htons(4242);
    if (inet_pton(AF_INET, "0.0.0.0", &sin.sin_addr.s_addr) == -1)
        return -1;

	serv_opts.listen_addrs[0] = *((struct sockaddr *)&sin);
	serv_opts.addr = strdup("10.0.0.101");

	/*errcode = tnt_parse_file(_PATH_DEFAULT_CONFIG_FILE);
	if (errcode == -1)
	{
		fprintf(stderr, "No conf file");
		exit(1);
	}*/

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
	log_err(-1, "Failed to init WSA.");
	}

	if ((evbase = event_base_new()) == NULL) {
	log_err(-1, "Failed to init the event library");
	} else {
		libevent_dump(evbase);
	}
	if ((interfce = tnt_ttc_open(TNT_TUNMODE_ETHERNET)) == NULL) {
		log_err(-1, "Failed to open a tap interface");
	}
	/*
	Arty: There's definitely a workaround for those signals on windows, we'll
	see that later.
	*/
    //sigterm = event_new(evbase, SIGTERM, EV_SIGNAL, &chld_sighdlr, evbase);
    //sigint = event_new(evbase, SIGINT, EV_SIGNAL, &chld_sighdlr, evbase);

    if (server_init(&server, evbase) == -1)
	log_err(-1, "Failed to init the server socket");

	if (tnt_ttc_set_ip(interfce, "10.0.0.101/24") == -1){
		log_err(-1, "Failed to set interface's ip");
	}
	if (tnt_ttc_up(interfce) != 0) {
		log_err(-1, "For some reason, the interface couldn't be up'd");
	}

	/*Arty: This fails... Badly.*/
	server_set_device(&server, tnt_ttc_get_fd(interfce));

    //event_add(sigterm, NULL);
    //event_add(sigint, NULL);

    log_info("tnetacle ready");


    log_info("Starting event loop");
	if (event_base_dispatch(evbase) == -1) {
		errcode = WSAGetLastError();
		fprintf(stderr, "(%d) %s\n", errcode, evutil_socket_error_to_string(errcode));
	}

    /*
     * It may look like we freed this one twice, once here and once in tnetacled.c
     * but this is not the case. Please don't erase this !
     */
    event_base_free(evbase);

	tnt_ttc_close(interfce);
    //event_free(sigterm);
    //event_free(sigint);

    log_info("tnetacle exiting");
	WSACleanup();
    return TNT_OK;
}