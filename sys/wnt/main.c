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
#include "hexdump.h"

int debug;
extern struct options serv_opts;

typedef struct IOData {
	HANDLE fd;
	struct server *server;
	char enabled;
} IODATA, *PIODATA;

IODATA IOCPData;

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
void broadcast_to_peers(struct server *s);
extern OVERLAPPED gl_overlap;

static int
async_read(HANDLE fd, void *buf, size_t len)
{
	DWORD n = 0;
	int err;

	err = ReadFile(fd, buf, len, &n, &gl_overlap);
	//log_debug("READ fd=%ld, buf=%p, len=%ld, n=%ld, err=%d", fd, buf, len, n, err);
	if (err == 0 && GetLastError() != ERROR_IO_PENDING)
	{
		printf("Read failed, error %d\n", GetLastError());
		return -1;
	}
	else
	{
		if (n > 0)
		{
			log_debug("== READ == READ == READ == READ ==");
			hex_dump_chk(buf, n);
			log_debug("== READ == READ == READ == READ ==");
		}
		WaitForSingleObject(gl_overlap.hEvent, 1000);
		return n;
	}
}

DWORD WINAPI IOCPFunc(void *lpParam)
{
	HANDLE hPort;
	IODATA data = *(PIODATA)(lpParam);
	DWORD n;
	ULONG_PTR dummy;
	OVERLAPPED oL;
	BOOL ret;
	int _n = 0;
    struct frame tmp;

	while (data.enabled == 1)
	{
		while ((n = async_read(data.fd, &tmp.frame, sizeof(tmp.frame))) > 0)
		{
			tmp.size = (unsigned short)n;/* We cannot read more than a ushort*/
			v_frame_push(&(data.server->frames_to_send), &tmp);
			//log_debug("Read a new frame of %d bytes.", n);
			_n++;
		}
		if (n == 0 || GetLastError() == ERROR_IO_PENDING)
		{
			//log_debug("Read %d frames in this pass.", _n);
			broadcast_to_peers(data.server);
		}
		else if (n == -1)
			log_warn("read on the device failed:");
	}
	return 0;
}

int
main(int argc, char *argv[]) {

	WSADATA wsaData;
	HANDLE hIOCPThread;
	DWORD dwThreadId;
	int errcode;
    struct event_base *evbase = NULL;
    struct server server;
	struct device *interfce;
	char cnf_file[2048];

	strcpy(cnf_file, getenv("APPDATA"));
	strncat(cnf_file, _PATH_DEFAULT_CONFIG_FILE, strlen(_PATH_DEFAULT_CONFIG_FILE));
	errcode = tnt_parse_file(cnf_file);
	if (errcode == -1)
	{
		fprintf(stderr, "No conf file");
		exit(1);
	}

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

	/* start iocp thread here. */
	IOCPData.fd = (HANDLE)tnt_ttc_get_fd(interfce);
	IOCPData.enabled = 1;
	IOCPData.server = &server;
	hIOCPThread = CreateThread(
		NULL,
		0,
		IOCPFunc,
		&IOCPData,
		0,
		&dwThreadId);
	if (hIOCPThread == NULL) {
		log_err(-1, TEXT("CreateThread for IOCP thread failed."));
	}


	if (tnt_ttc_set_ip(interfce, serv_opts.addr) == -1){
		log_err(-1, "Failed to set interface's ip");
	}
	if (tnt_ttc_up(interfce) != 0) {
		log_err(-1, "For some reason, the interface couldn't be up'd");
	}

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

	/* Additionnal IOCP thread collection */
	IOCPData.enabled = 0;
	WaitForSingleObject(hIOCPThread, INFINITE);
	CloseHandle(hIOCPThread);

	tnt_ttc_close(interfce);
    //event_free(sigterm);
    //event_free(sigint);

    log_info("tnetacle exiting");
	WSACleanup();
    return TNT_OK;
}