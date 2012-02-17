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

/* imsg specific includes */
#include <sys/uio.h>
#include <sys/queue.h>
#include <imsg.h>

#include <event2/event.h>

struct imsg_cb_data {
    struct imsgbuf *ibuf;
    struct event *ievent;
    struct event_base *evbase;
};

volatile sig_atomic_t chld_quit;

int tnt_dispatch_imsg(struct imsg_cb_data *);

static void
tnt_imsg_callback(evutil_socket_t fd, short events, void *args)
{
    (void)fd;
    struct imsg_cb_data *cb_data = args; 
    struct imsgbuf *ibuf = cb_data->ibuf;

    if (events & EV_READ) {
	tnt_dispatch_imsg(cb_data);
    }

    if (events & EV_WRITE) {
	if (ibuf->w.queued > 0) {
	    msgbuf_write(&ibuf->w);
	}
    }
}

static void
device_cb(evutil_socket_t fd, short events, void *args)
{
    (void)fd;
    (void)args;
    printf("Hello !\n");
    if (events & EV_READ)
	printf("READ EVENTS\n");
    if (events & EV_WRITE)
	printf("WRITE EVENTS\n");
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
	log_err(1, "[unpriv] can't drop privileges (setgroups)");
#ifdef HAVE_SETRESXID
    if (setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) == -1 ||
	setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid) == -1)
	log_err(1, "[unpriv] can't drop privileges (setresid)");
#else
    /* Fallback to setuid, but it might not work properly */
    if (setuid(pw->pw_uid) == -1 || setgid(pw->pw_gid) == -1)
	log_err(1, "[unpriv] can't drop privileges (setuid||setgid)");
#endif
}

int
tnt_fork(int imsg_fds[2], struct passwd *pw) {
    pid_t pid;
    struct imsgbuf ibuf;
    /* XXX: To remove when Mota will bring his network code */
    struct imsg_cb_data cb_data = { .ibuf = NULL, .ievent = NULL, .evbase = NULL};
    struct event *evimsg = NULL;
    struct event *sigterm = NULL;
    struct event *sigint = NULL;

    switch ((pid = fork())) {
    case -1:
        log_err(TNT_OSERR, "tnt_fork: ");
        break;
    case 0:
        tnt_setproctitle("[unpriv]");
        break;
    default:
        tnt_setproctitle("[priv]");
        return pid;
    }

    tnt_priv_drop(pw);

    if ((cb_data.evbase = event_base_new()) == NULL) {
	log_err(-1, "[unpriv] Failed to init the event library");
    }

    /*
       signal(SIGTERM, chld_sighdlr);
       signal(SIGINT, chld_sighdlr);
     */
    sigterm = event_new(cb_data.evbase, SIGTERM, EV_SIGNAL, &chld_sighdlr,
      cb_data.evbase);
    sigint = event_new(cb_data.evbase, SIGINT, EV_SIGNAL, &chld_sighdlr,
      cb_data.evbase);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);

    if (close(imsg_fds[0]))
	log_notice("[unpriv] close");

    imsg_init(&ibuf, imsg_fds[1]);
    cb_data.ibuf = &ibuf;
    evimsg = event_new(cb_data.evbase, imsg_fds[1],
      EV_READ | EV_WRITE | EV_ET | EV_PERSIST,
      &tnt_imsg_callback, &cb_data);

    event_add(evimsg, NULL);
    event_add(sigterm, NULL);
    event_add(sigint, NULL);

    log_info("[unpriv] tnetacle ready");

    /* Immediately request the creation of a tun interface */
    imsg_compose(&ibuf, IMSG_CREATE_DEV, 0, 0, -1, NULL, 0);

    event_base_dispatch(cb_data.evbase);
    /* cleanely exit */
    msgbuf_write(&ibuf.w);
    msgbuf_clear(&ibuf.w);

    /*
     * It may look like we freed this one twice, once here and once in tnetacled.c
     * but this is not the case. Please don't erase this !
     */
    event_base_free(cb_data.evbase);
    event_free(evimsg);
    event_free(sigterm);
    event_free(sigint);

    log_info("[unpriv] tnetacle exiting");
    exit(TNT_OK);
}

/*
 * The purpose of this function is to handle requests sent by the
 * root level process.
 */
int
tnt_dispatch_imsg(struct imsg_cb_data *data) {
    struct imsg imsg;
    ssize_t n;
    int tun_fd;
    struct imsgbuf *ibuf = data->ibuf;

    n = imsg_read(ibuf);
    if (n == -1) {
	log_warnx("[unpriv] loose some imsgs");
	imsg_clear(ibuf);
	return 0;
    }

    if (n == 0) {
	log_warnx("[unpriv] pipe closed");
	return -1;
    }

    /* Loops through the queue created by imsg_read */
    while ((n = imsg_get(ibuf, &imsg)) != 0 && n != -1) {
	switch (imsg.hdr.type) {
        case IMSG_CREATE_DEV:
            if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(tun_fd))
                log_errx(1, "[unpriv] invalid IMSG_CREATE_DEV received");
            (void)memcpy(&tun_fd, imsg.data, sizeof tun_fd);
            log_info("[unpriv] receive IMSG_CREATE_DEV: fd %i", tun_fd);
            data->ievent = event_new(data->evbase, tun_fd,
            			 EV_WRITE | EV_READ | EV_ET | EV_PERSIST,
            			 &device_cb, &data);
            event_add(data->ievent, NULL);
        
            /* directly ask to configure the tun device */
        
            imsg_compose(ibuf, IMSG_SET_IP, 0, 0, -1,
            	     TNETACLE_LOCAL_ADDR, strlen(TNETACLE_LOCAL_ADDR));
            imsg_compose(ibuf, IMSG_SET_NETMASK, 0, 0, -1,
            	     "255.255.255.0", strlen("255.255.255.0"));
            break;
        default:
            break;
        }
	imsg_free(&imsg);
    }
    if (n == -1) {
	log_warnx("[unpriv] imsg_get");
	return -1;
    }
    return 0;
}

