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
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#ifdef Linux
# include <linux/if.h>
# include <linux/if_tun.h>
#else
# include <net/if.h>
# ifndef Darwin
#  include <net/if_tun.h>
# endif
#endif

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tnetacle.h"
#include "tntexits.h"
#include "log.h"
#include "tun.h"

/* imsg specific includes */
#include <sys/uio.h>
#include <sys/queue.h>
#include <imsg.h>

#include <event2/event.h>

int debug;
volatile sig_atomic_t sigchld_recv;

/* XXX: clean that after the TA2 */
struct device *dev = NULL;

static void usage(void);
static int dispatch_imsg(struct imsgbuf *);

struct imsg_data {
    struct imsgbuf *ibuf;
    struct event *event;
    struct event_base *evbase;
    int is_ready_read;
    int is_ready_write;
};

static void
_sighdlr(int sig) {
    printf("%s\n", __PRETTY_FUNCTION__);
    if (sig == SIGCHLD)
	sigchld_recv = 1;
}

static void
sighdlr(evutil_socket_t sig, short events, void *args) {
    struct event_base *evbase = args;
    (void)events;

    switch (sig) {
    case SIGCHLD:
    case SIGTERM:
    case SIGINT:
	log_warn("Received signal %d, stoping.\n", sig);
	event_base_loopbreak(evbase);
	break;
    /* TODO: SIGHUP */
    }
}

static void
imsg_callback_handler(evutil_socket_t fd, short events, void *args) {
    (void)fd;
    struct imsg_data *data = args;

    if (events & EV_READ || data->is_ready_read == 1) {
	data->is_ready_read = 1;
	if (dispatch_imsg(data->ibuf) == -1)
	    data->is_ready_read = 0;
    }

    if (events & EV_WRITE || data->is_ready_write == 1) {
	data->is_ready_write = 1;
	if (data->ibuf->w.queued > 0) {
	    if (msgbuf_write(&data->ibuf->w) == -1)
		data->is_ready_write = 0;
	}
    }
    return ;
}

int
main(int argc, char *argv[]) {
    int ch;
    pid_t chld_pid;
    struct passwd *pw;
    int imsg_fds[2];
    struct imsgbuf ibuf;
    struct imsg_data data;
    struct event_base *evbase;
    struct event *event = NULL;
    struct event *sigint = NULL;
    struct event *sigterm = NULL;
    struct event *sigchld = NULL;

    /* Parse configuration file and then command line switches */
    tnt_parse_file(NULL);

    while ((ch = getopt(argc, argv, "dhD:f:")) != -1) {
        switch(ch) {
        case 'd':
            debug = 1;
        break;
        case 'D':
            /*TODO */
            /*tnt_parse_line(optarg);*/
        break;
        case 'f':
            tnt_parse_file(optarg);
        break;
        case 'h':
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;

    log_init();

    if (geteuid()) {
	(void)fprintf(stderr, "need root privileges\n");
	return 1;
    }

    if ((pw = getpwnam(TNETACLE_USER)) == NULL) {
	(void)fprintf(stderr, "unknown user " TNETACLE_USER "\n");
	return TNT_NOUSER;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, imsg_fds) == -1) {
	perror("socketpair");
	return 1;
    }

    if (debug == 0) {
	if (tnt_daemonize() == -1) {
		fprintf(stderr, "can't daemonize\n");
		return 1;
	}
    }
    /* The child can die while we are still in the init phase. So we need to
     * monitor for SIGCHLD by signal
     */
    signal(SIGCHLD, _sighdlr);
    chld_pid = tnt_fork(imsg_fds, pw);

    if ((evbase = event_base_new()) == NULL) {
	log_err(-1, "Failed to init the event library");
    }

    sigint = event_new(evbase, SIGTERM, EV_SIGNAL, &sighdlr, evbase);
    sigterm = event_new(evbase, SIGTERM, EV_SIGNAL, &sighdlr, evbase);
    sigchld = event_new(evbase, SIGCHLD, EV_SIGNAL, &sighdlr, evbase);

    if (close(imsg_fds[1]))
	log_notice("close");

    data.evbase = evbase;
    data.is_ready_write = 0;
    data.is_ready_read = 0;
    data.ibuf = &ibuf;
    imsg_init(&ibuf, imsg_fds[0]);
    evutil_make_socket_nonblocking(imsg_fds[0]);
    event = event_new(evbase, imsg_fds[0],
		      EV_READ | EV_WRITE | EV_ET | EV_PERSIST,
		      &imsg_callback_handler, &data);
    data.event = event;

    event_add(event, NULL);
    event_add(sigint, NULL);
    event_add(sigterm, NULL);
    event_add(sigchld, NULL);

    /* if we received a sigchild from here, we don't need to start the event loop
     * as the child is stillborn.. :(
     */
    if (sigchld_recv != 1)
	event_base_dispatch(evbase);
    else
	fprintf(stderr, "The tNETacle initialisation phase failed."
		" Check the logs to find out why.");
    
    signal(SIGCHLD, SIG_DFL);

    if (chld_pid != 0)
	kill(chld_pid, SIGTERM);

    msgbuf_clear(&ibuf.w);
    event_base_free(evbase);
    event_free(event);
    event_free(sigint);
    event_free(sigterm);
    event_free(sigchld);
    log_info("tnetacle exiting");
    return TNT_OK;
}

static void
usage(void) {
    char *progname = tnt_getprogname();

    (void)fprintf(stderr, "%s: [-dh]\n", progname);
    exit(TNT_USAGE);
}

static int
dispatch_imsg(struct imsgbuf *ibuf) {
    struct imsg imsg;
    ssize_t n;
    ssize_t datalen;
    int fd;
    char buf[128];

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

    for (;;) {
	/* Loops through the queue created by imsg_read */
	n = imsg_get(ibuf, &imsg);
	if (n == -1) {
	    log_warnx("imsg_get");
	    return -1;
	}

	/* Nothing was ready, return to the main loop */
	if (n == 0)
	    break;

	switch (imsg.hdr.type) {
	case IMSG_CREATE_DEV:
	    dev = tnt_ttc_open();
	    fd = tnt_ttc_get_fd(dev);
	    imsg_compose(ibuf, IMSG_CREATE_DEV, 0, 0, -1,
	      &fd, sizeof(int));
	    break;
	case IMSG_SET_IP:
	    if (dev == NULL) {
	        log_warnx("can't set ip, use IMSG_CREATE_DEV first");
	        break;
	    }
	    datalen = imsg.hdr.len - IMSG_HEADER_SIZE;
	    (void)memset(buf, '\0', sizeof buf);
	    (void)memcpy(buf, imsg.data, sizeof buf);
	    buf[datalen] = '\0';
	    
	    log_info("receive IMSG_SET_IP: %s", buf);
	    tnt_ttc_set_ip(dev, buf);
	    break;
	default:
	    break;
	}
	imsg_free(&imsg);
    }
    return 0;
}

