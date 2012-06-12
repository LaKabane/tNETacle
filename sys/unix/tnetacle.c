/*
 * Copyright (c) 2011 Tristan Le Guern <leguern AT medu DOT se>
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

#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>

#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tntexits.h"
#include "tnetacle.h"
#include "log.h"
#include "options.h"
#include "server.h"

/* imsg specific includes */
#include <sys/uio.h>
#include <sys/queue.h>
#include <imsg.h>

#include <event2/event.h>

extern struct options server_options;

struct imsg_data {
    struct imsgbuf *ibuf;
    struct event_base *evbase;
    struct server *server;
    int is_ready_read;
    int is_ready_write;
};

volatile sig_atomic_t chld_quit;

int tnt_dispatch_imsg(struct imsg_data *);

static void
tnt_imsg_callback(evutil_socket_t fd, short events, void *args) {
    (void)fd;
    struct imsg_data *data = args; 
    struct imsgbuf *ibuf = data->ibuf;

    if (events & EV_READ || data->is_ready_read == 1) {
	data->is_ready_read = 1;
	if (tnt_dispatch_imsg(data) == -1)
	    data->is_ready_read = 0;
    }
    if (events & EV_WRITE || data->is_ready_write == 1) {
	data->is_ready_write = 1;
	if (ibuf->w.queued > 0) {
	    if (msgbuf_write(&ibuf->w) == -1)
		data->is_ready_write = 0;
	}
    }
}

void
device_cb(evutil_socket_t fd, short events, void *args)
{
    (void)fd;
    (void)args;
    printf("%s\n", __PRETTY_FUNCTION__);
    if (events & EV_READ) {
	printf("%s::[READ EVENTS]\n", __PRETTY_FUNCTION__);
    }
    if (events & EV_WRITE) {
	printf("%s::[WRITE EVENTS]\n", __PRETTY_FUNCTION__);
    }
}

static void
chld_sighdlr(evutil_socket_t sig, short events, void *args) {
    (void)events;
    struct event_base *evbase = args;

    switch (sig) {
	case SIGTERM:
	case SIGINT:
	    event_base_loopbreak(evbase);
	    break;
    }
}

static void
tnt_priv_drop(struct passwd *pw) {
    struct stat ss; /* Heil! */

    /* All this part is a preparation to the privileges dropping */
    if (stat(pw->pw_dir, &ss) == -1)
	log_err(1, "stat");
    if (ss.st_uid != 0 || (ss.st_mode & (S_IWGRP | S_IWOTH)) != 0)
	log_errx(1, "bad permissions");
    if (chroot(pw->pw_dir) == -1)
	log_err(1, "chroot");
    if (chdir("/") == -1)
	log_err(1, "chdir(\"/\")");
    /* TODO: dup stdin, stdout and stdlog_err to /dev/null */

    if (setgroups(1, &pw->pw_gid) == -1)
	log_err(1, "can't drop privileges (setgroups)");
#ifdef HAVE_SETRESXID
    if (setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) == -1 ||
	setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid) == -1)
	log_err(1, "can't drop privileges (setresid)");
#else
    /* Fallback to setuid, but it might not work properly */
    if (setuid(pw->pw_uid) == -1 || setgid(pw->pw_gid) == -1)
	log_err(1, "can't drop privileges (setuid||setgid)");
#endif
}

static struct event *
init_pipe_endpoint(int imsg_fds[2], 
		   struct imsg_data *data) {

    struct event *event = NULL;

    if (close(imsg_fds[0]))
	log_notice("close");

    data->is_ready_write = 0;
    data->is_ready_read = 0;
    imsg_init(data->ibuf, imsg_fds[1]);
    evutil_make_socket_nonblocking(imsg_fds[1]);
    event = event_new(data->evbase, imsg_fds[1],
		      EV_READ | EV_WRITE | EV_ET | EV_PERSIST,
		      &tnt_imsg_callback, data);
    return event;
}

int
tnt_fork(int imsg_fds[2], struct passwd *pw) {
    pid_t pid;
    struct imsgbuf ibuf;
    /* XXX: To remove when Mota will bring his network code */
    struct imsg_data data;
    struct event_base *evbase = NULL;
    struct event *sigterm = NULL;
    struct event *sigint = NULL;
    struct event *imsg_event = NULL;
    struct server server;

    switch ((pid = fork())) {
    case -1:
	log_err(TNT_OSERR, "tnt_fork: ");
	break;
    case 0:
	tnt_setproctitle("[unpriv]");
	log_set_prefix("unpriv");
	break;
    default:
	tnt_setproctitle("[priv]");
	log_set_prefix("priv");
	return pid;
    }

    tnt_priv_drop(pw);

    if ((evbase = event_base_new()) == NULL) {
	log_err(-1, "Failed to init the event library");
    }

    sigterm = event_new(evbase, SIGTERM, EV_SIGNAL, &chld_sighdlr, evbase);
    sigint = event_new(evbase, SIGINT, EV_SIGNAL, &chld_sighdlr, evbase);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);

    if (server_init(&server, evbase) == -1)
	log_err(-1, "Failed to init the server socket");

    data.ibuf = &ibuf;
    data.evbase = evbase;
    data.server = &server;
    imsg_event = init_pipe_endpoint(imsg_fds, &data);

    event_add(sigterm, NULL);
    event_add(sigint, NULL);
    event_add(imsg_event, NULL);

    log_info("tnetacle ready");

    /* Immediately request the creation of a tun interface */
    imsg_compose(&ibuf, IMSG_CREATE_DEV, 0, 0, -1, NULL, 0);

    log_info("Starting event loop");
    event_base_dispatch(evbase);

    /* cleanely exit */
    msgbuf_write(&ibuf.w);
    msgbuf_clear(&ibuf.w);

    /*
     * It may look like we freed this one twice, once here and once in tnetacled.c
     * but this is not the case. Please don't erase this !
     */
    event_base_free(evbase);

    event_free(sigterm);
    event_free(sigint);
    event_free(imsg_event);

    log_info("tnetacle exiting");
    exit(TNT_OK);
}

/*
 * The purpose of this function is to handle requests sent by the
 * root level process.
 */
int
tnt_dispatch_imsg(struct imsg_data *data) {
    struct imsg imsg;
    ssize_t n;
    int tun_fd;
    struct imsgbuf *ibuf = data->ibuf;
    struct event_base *evbase = data->evbase;
    struct event *ievent = NULL;

    n = imsg_read(ibuf);
    if (n == -1) {
	log_warnx("loose some imsgs");
	imsg_clear(ibuf);
	return -1;
    }

    if (n == 0) {
	log_warnx("pipe closed");
	return -1;
    }

    /* Loops through the queue created by imsg_read */
    while ((n = imsg_get(ibuf, &imsg)) != 0 && n != -1) {
	switch (imsg.hdr.type) {
	case IMSG_CREATE_DEV:
	    if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(tun_fd))
		log_errx(1, "invalid IMSG_CREATE_DEV received");
	    (void)memcpy(&tun_fd, imsg.data, sizeof tun_fd);
	    log_info("receive IMSG_CREATE_DEV: fd %i", tun_fd);
	    ievent = event_new(evbase, tun_fd,
			       EV_READ | EV_PERSIST,
			       &device_cb, &data);
	    event_add(ievent, NULL);

	    server_set_device(data->server, ievent);

	    /* directly ask to configure the tun device */

	    imsg_compose(ibuf, IMSG_SET_IP, 0, 0, -1,
			server_options.addr , strlen(server_options.addr));
	    break;
	default:
	    break;
	}
	imsg_free(&imsg);
    }
    if (n == -1) {
	log_warnx("imsg_get");
	return -1;
    }
    return 0;
}

