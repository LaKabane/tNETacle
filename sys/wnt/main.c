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
#include <ws2def.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>

#include <io.h>

#include <tuntap.h>

#include "tnetacle.h"
#include "tntexits.h"
#include "log.h"
#include "options.h"
#include "pathnames.h"
#include "hexdump.h"
#include "wincompat.h"
#include "server.h"
#include "frame.h"
#include "udp.h"

int debug;
extern struct options serv_opts;

typedef struct IOData {
    HANDLE fd;
    SOCKET pipe_fd;
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

void handle_frames(char *frame, size_t size, LPOVERLAPPED_ENTRY Ol, struct server *s, SOCKET pipe_fd)
{
    char *packet;
    short frame_size = (short)size;
    int errcode;
    size_t packet_size = frame_size + sizeof(frame_size);
    //LPOVERLAPPED overlapped = NULL;

    /*
     * Here we create by hand a packet to send to readed frame to the other thread.
     */
    packet = (char *)malloc(size + sizeof(short));
    if (packet == NULL)
        return ;
    (void)memcpy(packet, &frame_size, sizeof(frame_size));
    (void)memcpy(packet + sizeof(frame_size), frame, frame_size);

    //The send is supposed to be async also, so we need a new overlapped
    //overlapped = (LPOVERLAPPED)malloc(sizeof(OVERLAPPED));
    //if (overlapped != NULL)
    //{
    //    int errcode;
    //    //WSABUF buf[1];
    //    /*
    //     * Here, it's tricky. I'm supposed to send the data to a socket created
    //     * with WSASocket. However, the call to WriteFile doesn't work, neither
    //     * the call to WSASend. So I ended up using send, but this is clearly
    //     * not a good solution.
    //     * If someone know how to correct this, feel free to do so ! And contact
    //     * me at pichot.fabien AT gmail.com to explain me what the f*ck was the
    //     * solution.
    //     */
    //    memset(overlapped, 0, sizeof(OVERLAPPED));
    //    //buf[0].buf = packet;
    //    //buf[0].len = packet_size;
    //    //errcode = WriteFile((HANDLE)pipe_fd, packet, packet_size, NULL, overlapped);
    //    //errcode = WSASend((SOCKET)pipe_fd, buf, 1, NULL, 0, overlapped, NULL);
    //    errcode = send(pipe_fd, packet, packet_size, 0);
    //    if (errcode == 0 && (errcode = WSAGetLastError()) != WSA_IO_PENDING)
    //    {
    //        log_debug("Error(%d) pipe write: %s", errcode, formated_error(L"%1%0", errcode));
    //    }
    //}

    errcode = send(pipe_fd, packet, (int)packet_size, 0);
    if (errcode == 0 && (errcode = WSAGetLastError()) != WSA_IO_PENDING)
    {
        log_debug("Error(%d) pipe write: %s", errcode, formated_error(L"%1%0", errcode));
    }

}

void pipe_read(struct evbuffer *evb, LPOVERLAPPED_ENTRY Ol, HANDLE device_fd)
{
    unsigned short *sptr;
    void *frame_ptr;
    short frame_size;
    LPOVERLAPPED overlapped = NULL;

    log_debug("evbuffer_get_length(evb) = %d", evbuffer_get_length(evb));
    /* As long as the buffer in not empty*/
    while (evbuffer_get_length(evb) > 0)
    {
        int errcode;

        /*
         * We will need a new overlapped struct, because the write to the
         * device_fd is also async.
         */
        overlapped = (LPOVERLAPPED)malloc(sizeof(OVERLAPPED));
        if (overlapped == NULL)
            break;
        memset(overlapped, 0, sizeof(*overlapped));

        /* We peak the size from the buffer */
        sptr = (unsigned short *)evbuffer_pullup(evb, sizeof(*sptr));
        if (sptr == NULL) /* this is not supposed to happen */
            break;
        frame_size = *sptr;
        /* We are going to drain just after, so we need to count 2 bytes less*/
        if (frame_size > (evbuffer_get_length(evb) - sizeof *sptr))
        {
            log_debug("Have a frame of size %d, but don't have enought data (%d)", frame_size,
                evbuffer_get_length(evb));
            break;
        }
        evbuffer_drain(evb, sizeof(*sptr));
        frame_ptr = evbuffer_pullup(evb, frame_size);
        errcode = WriteFile(device_fd, frame_ptr, frame_size, NULL, overlapped);
        if (errcode != 0)
        {
            log_debug("Write %d on the device", frame_size);
        }
        else if ((errcode = GetLastError()) != ERROR_IO_PENDING)
        {
            log_debug("Error device(%p) write: (%d) %s", device_fd,
                errcode, formated_error(L"%1%0", errcode));
        }
        /* Don't forget to drain the frame we wrote from the buffer... */
        evbuffer_drain(evb, frame_size);
    }
}

/*
 * Well, this function might be the most horrible chunk of code I've ever wrote.
 * I know nothing about ICOP and WinAPI to begin with, sorry :(.
 * So, to you, adventurous reader, take care, you're entering the dragon's cave..
 */

/*
 * In the other hand, if someone know how to do it right, please contact me at
 * pichot.fabien AT gmail.com !
 */

DWORD IOCPFunc(void *lpParam)
{
    IODATA data = *(PIODATA)(lpParam);
    HANDLE hPort;
    LPOVERLAPPED deviceOl;
    LPOVERLAPPED pipeOl;
    char device_buf[2048];
    char pipe_buf[2048];
    struct evbuffer *evb_pipe;
    OVERLAPPED_ENTRY entries[10];
    BOOL iocp_status;
    BOOL io_status;
    BOOL need_new_dev_overlapped = 1;
    BOOL need_new_pipe_overlapped = 1;
    size_t n = 0;
    int errcode;

    evb_pipe = evbuffer_new();
    /*
     * We create two new completion port, the first call create it and the
     * second one just add the HANDLE to the CP.
     * The magic numbers here are used to identify the HANDLE later
     */
    hPort = CreateIoCompletionPort(data.fd, NULL, 0xDEADDEAD, 0);
    if (hPort == NULL)
        log_notice("Error: %s", formated_error(L"%1", GetLastError()));
    hPort = CreateIoCompletionPort((HANDLE)data.pipe_fd, hPort, 0xDEADBEAF, 0);
    if (hPort == NULL)
        log_notice("Error: %s", formated_error(L"%1", GetLastError()));
    /*Forever*/
    for (;;)
    {
        memset(entries, 0, sizeof(entries));

        /* If the previous overlapped operation endend*/
        if (need_new_dev_overlapped == 1)
        {
            /* We alloc a new overlapped struct, because we need one per call*/
            deviceOl = (LPOVERLAPPED)malloc(sizeof(OVERLAPPED));
            if (deviceOl == NULL)
            {
                log_notice("Memory Failure");
                return 0;
            }
            memset(deviceOl, 0, sizeof(*deviceOl));

            /* We do the ReadFile, with the previously allocated Overlapped.*/
            io_status = ReadFile(data.fd, device_buf, sizeof(device_buf), NULL, deviceOl);
            if (io_status == 0 && (errcode = GetLastError()) != ERROR_IO_PENDING)
            {
                log_notice("ReadFile on device Error: %s", formated_error(L"%1", errcode));
            }
            /* And we set this one to 0, until this overlapped operation is finished*/
            need_new_dev_overlapped = 0;
        }
        if (need_new_pipe_overlapped == 1)
        {
            /* We alloc a new overlapped struct, because we need one per call*/
            pipeOl = (LPOVERLAPPED)malloc(sizeof(OVERLAPPED));
            if (pipeOl == NULL)
            {
                log_notice("Memory Failure");
                return 0;
            }
            memset(pipeOl, 0, sizeof(*pipeOl));

            /* We do the ReadFile, with the previously allocated Overlapped.*/
            io_status = ReadFile((HANDLE)data.pipe_fd, pipe_buf, sizeof(pipe_buf), NULL, pipeOl);
            if (io_status == 0 && (errcode = GetLastError()) != ERROR_IO_PENDING)
            {
                log_notice("ReadFile on pipe Error: %s", formated_error(L"%1", errcode));
            }
            /* And we set this one to 0, until this overlapped operation is finished*/
            need_new_pipe_overlapped = 0;
        }

        /* This call will blocked until an overlapped operation finish.*/
        /* The result will be put in the 'entries' array, and 'n' contain the number of entry*/
        iocp_status = GetQueuedCompletionStatusEx(hPort, entries, sizeof(entries) / sizeof(*entries), &n, INFINITE, FALSE);
        if (iocp_status == 0)
        {
            log_notice("Queue Error: %s", formated_error(L"%1", GetLastError()));
        }
        else
        {
            int i = 0;
            /* For all received entries*/
            for (; i < n; i++)
            {
                OVERLAPPED_ENTRY *current = &entries[i];
                int key = current->lpCompletionKey;

                /*Event on the device*/
                if (key == 0xDEADDEAD)
                {
                    DWORD size;

                    /*
                     * Here we fetch the interesting data from the entry, call the
                     * function that will send the new frame to the other thread, and
                     * free the OVERLAPPED struct.
                     */
                    size = current->dwNumberOfBytesTransferred;
                    handle_frames(device_buf, size, current, data.server, data.pipe_fd);
                    free(current->lpOverlapped);
                    /*
                     * We put this one to 1, to signal to the main loop that this operation
                     * is finished, and that its time to start a new overlapped operation.
                     */
                    need_new_dev_overlapped = 1;
                }
                /*Event on the pipe*/
                else if (key == 0xDEADBEAF)
                {
                    DWORD size;

                    /*
                     * Here we fetch the interesting data from the entry, and
                     * put the data we just read in an evbuffer.
                     * This is usefull because we don't always read full frames
                     * on the pipe.
                     */
                    size = current->dwNumberOfBytesTransferred;
                    evbuffer_add(evb_pipe, pipe_buf, size);
                    pipe_read(evb_pipe, current, data.fd);
                    free(current->lpOverlapped);
                    /*
                     * We put this one to 1, to signal to the main loop that this operation
                     * is finished, and that its time to start a new overlapped operation.
                     */
                    need_new_pipe_overlapped = 1;
                }
            }
        }
    }
    return 0;
}

static void
free_frame(struct frame const *f)
{
    free(f->raw_packet);
}

static void
pipe_read_cb(struct bufferevent *bev, void *data)
{
    struct server *s = (struct server *)data;
    struct evbuffer *input = bufferevent_get_input(bev);
    struct frame tmp;

    while (evbuffer_get_length(input) > 0)
    {
        short *size_ptr = (short *)evbuffer_pullup(input, sizeof(short));
        short frame_size = *size_ptr;
        unsigned char *frame_ptr;

        if (frame_size > evbuffer_get_length(input))
            break ;
        
        evbuffer_drain(input, sizeof(short));
        frame_alloc(&tmp, *size_ptr);
        frame_ptr = evbuffer_pullup(input, frame_size);
        memcpy(tmp.frame, frame_ptr, frame_size);
        tmp.size = frame_size;

        v_frame_push(s->frames_to_send, &tmp);
        evbuffer_drain(input, frame_size);
    }
    broadcast_udp_to_peers(s);
    v_frame_foreach(s->frames_to_send, free_frame);
    v_frame_clean(s->frames_to_send);
}

int
tnt_socketpair(int family, int type, int protocol,
    evutil_socket_t fd[2])
{
	/* This code is originally from Tor.  Used with permission. */

	/* This socketpair does not work when localhost is down. So
	 * it's really not the same thing at all. But it's close enough
	 * for now, and really, when localhost is down sometimes, we
	 * have other problems too.
	 */
#ifdef WIN32
#define ERR(e) WSA##e
#else
#define ERR(e) e
#endif
	evutil_socket_t listener = -1;
	evutil_socket_t connector = -1;
	evutil_socket_t acceptor = -1;
	struct sockaddr_in listen_addr;
	struct sockaddr_in connect_addr;
	ev_socklen_t size;
	int saved_errno = -1;

	if (protocol
		|| (family != AF_INET
#ifdef AF_UNIX
		    && family != AF_UNIX
#endif
		)) {
		EVUTIL_SET_SOCKET_ERROR(ERR(EAFNOSUPPORT));
		return -1;
	}
	if (!fd) {
		EVUTIL_SET_SOCKET_ERROR(ERR(EINVAL));
		return -1;
	}

    listener = WSASocket(AF_INET, type, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (listener == INVALID_SOCKET)
		return -1;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	listen_addr.sin_port = 0;	/* kernel chooses port.	 */
	if (bind(listener, (struct sockaddr *) &listen_addr, sizeof (listen_addr))
		== -1)
		goto tidy_up_and_fail;
	if (listen(listener, 1) == -1)
		goto tidy_up_and_fail;

    connector = WSASocket(AF_INET, type, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (connector == INVALID_SOCKET)
		goto tidy_up_and_fail;
	/* We want to find out the port number to connect to.  */
	size = sizeof(connect_addr);
	if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1)
		goto tidy_up_and_fail;
	if (size != sizeof (connect_addr))
		goto abort_tidy_up_and_fail;
	if (connect(connector, (struct sockaddr *) &connect_addr,
				sizeof(connect_addr)) == -1)
		goto tidy_up_and_fail;

	size = sizeof(listen_addr);
    acceptor = WSAAccept(listener, (struct sockaddr *) &listen_addr, &size, NULL, NULL);
    if (acceptor == INVALID_SOCKET)
		goto tidy_up_and_fail;
	if (size != sizeof(listen_addr))
		goto abort_tidy_up_and_fail;
	evutil_closesocket(listener);
	/* Now check we are talking to ourself by matching port and host on the
	   two sockets.	 */
	if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) == -1)
		goto tidy_up_and_fail;
	if (size != sizeof (connect_addr)
		|| listen_addr.sin_family != connect_addr.sin_family
		|| listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr
		|| listen_addr.sin_port != connect_addr.sin_port)
		goto abort_tidy_up_and_fail;
	fd[0] = connector;
	fd[1] = acceptor;

	return 0;

 abort_tidy_up_and_fail:
	saved_errno = ERR(ECONNABORTED);
 tidy_up_and_fail:
	if (saved_errno < 0)
		saved_errno = EVUTIL_SOCKET_ERROR();
	if (listener != -1)
		evutil_closesocket(listener);
	if (connector != -1)
		evutil_closesocket(connector);
	if (acceptor != -1)
		evutil_closesocket(acceptor);

	EVUTIL_SET_SOCKET_ERROR(saved_errno);
	return -1;
#undef ERR
}

int
main(int argc, char *argv[])
{
    WSADATA wsaData;
    HANDLE hIOCPThread;
    DWORD dwThreadId;
    int errcode;
    struct event_base *evbase = NULL;
    struct server server;
    struct device *tuntap;
    struct bufferevent *bev;
    struct event_config *cfg = event_config_new();
    char cnf_file[2048];
    evutil_socket_t pair[2];

	/* Parse the default configuration file */
    (void)strcpy(cnf_file, getenv("APPDATA"));
    (void)strncat(cnf_file, _PATH_DEFAULT_CONFIG_FILE, strlen(_PATH_DEFAULT_CONFIG_FILE));
    errcode = tnt_parse_file(cnf_file);
    if (errcode == -1) {
        log_err(1, "Failed to find the configuration file");
    }

	/* Initialize various libraries */
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        log_err(1, "Failed to init WSA");
    }
	if ((evbase = event_base_new_with_config(cfg)) == NULL) {
        log_err(1, "Failed to init the event library");
    } else {
        libevent_dump(evbase);
    }

	/* Initialize our code */
    if (serv_opts.encryption) {
        server.server_ctx = evssl_init();
	} else {
        server.server_ctx = NULL;
	}
	/* TODO: Get the wanted device type from the conf */
	if ((tuntap = tuntap_init()) == NULL) {
		log_err(1, "Failed to allocate a tap interface");
	}
	if (tuntap_start(tuntap, TUNTAP_MODE_ETHERNET, TUNTAP_ID_ANY) == -1) {
		tuntap_destroy(tuntap);
		log_err(1, "Failed to open a tap interface");
	}

    /*
     * Arty: There's definitely a workaround for those signals on windows, we'll
     * see that later.
     */
    //sigterm = event_new(evbase, SIGTERM, EV_SIGNAL, &chld_sighdlr, evbase);
    //sigint = event_new(evbase, SIGINT, EV_SIGNAL, &chld_sighdlr, evbase);

    errcode = evutil_socketpair(AF_INET, SOCK_STREAM, 0, &pair);
    if (errcode == -1) {
        log_notice("Failed to create the socketpair");
    }

    bev = bufferevent_socket_new(evbase, pair[0], BEV_OPT_CLOSE_ON_FREE);
    if (bev != NULL) {
        bufferevent_setcb(bev, pipe_read_cb, NULL, NULL, &server);
        bufferevent_enable(bev, EV_READ);
        server.pipe_endpoint = bev;
    }
    if (server_init(&server, evbase) == -1) {
        log_err(1, "Failed to initialize the server socket");
	}

    /* start iocp thread here */
    IOCPData.fd = (HANDLE)TUNTAP_GET_FD(tuntap);
    IOCPData.pipe_fd = pair[1];
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
        log_err(1, TEXT("Failed to create the IOCP thread"));
    }

	/* Now we can use the TAP32 driver */
	{
		char *ip, *mask;
		int netbits;
		int ret;

		ret = 0;
		ip = strdup(serv_opts.addr);
		if (ip == NULL) {
			log_err(1, "Invalid 'Address' value in the configuration file");
		}

		mask = strchr(ip, '/');
		if (mask == NULL) {
	        free(ip);
			log_err(1, "Invalid 'Address' value in the configuration file");
	    }
		*mask= '\0';
		++mask;

		netbits = (short)evutil_strtoll(mask, NULL, 10);
		ret = tuntap_set_ip(tuntap, ip, netbits);
		free(ip);
	}
	if (tuntap_up(tuntap) != 0) {
	    log_err(1, "Failed to connect the interface");
	}
    server_set_device(&server, (int)TUNTAP_GET_FD(tuntap));

    //event_add(sigterm, NULL);
    //event_add(sigint, NULL);

    log_info("tNETacle ready");

    log_info("Starting event loop");
    if (event_base_dispatch(evbase) == -1) {
        errcode = WSAGetLastError();
        fprintf(stderr, "(%d) %s\n", errcode, evutil_socket_error_to_string(errcode));
    }

    event_base_free(evbase);

    /* Additionnal IOCP thread collection */
    IOCPData.enabled = 0;
    WaitForSingleObject(hIOCPThread, INFINITE);
    CloseHandle(hIOCPThread);

	tuntap_down(tuntap);
    tuntap_destroy(tuntap);
    //event_free(sigterm);
    //event_free(sigint);

    log_info("tNETacle exiting");
    WSACleanup();
    return TNT_OK;
}
