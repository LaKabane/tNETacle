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
# include <net/if_tun.h>
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

int debug;
volatile sig_atomic_t sigchld;
volatile sig_atomic_t quit;

/* XXX: clean that after the TA2 */
struct device *dev = NULL;

static void usage(void);
static int dispatch_imsg(struct imsgbuf *);

void
sighdlr(int sig) {
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		quit = 1;
		break;
	case SIGCHLD:
		sigchld = 1;
		break;
	/* TODO: SIGHUP */
	}
}

int
main(int argc, char *argv[]) {
	int ch;
	pid_t chld_pid;
	struct passwd *pw;
	int imsg_fds[2];
	struct imsgbuf ibuf;
	/* XXX: To remove when Mota will bring his network code */
	fd_set masterfds;
	int fd_max;

	while ((ch = getopt(argc, argv, "dh")) != -1) {
		switch(ch) {
		case 'd':
			debug = 1;
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

	signal(SIGCHLD, sighdlr);
	chld_pid = tnt_fork(imsg_fds, pw);

	signal(SIGTERM, sighdlr);
	signal(SIGINT, sighdlr);

	if (close(imsg_fds[1]))
		log_notice("[priv] close");

	imsg_init(&ibuf, imsg_fds[0]);

	fd_max = ibuf.fd;
	FD_ZERO(&masterfds);
	FD_SET(ibuf.fd, &masterfds);

	while (quit == 0) {
		int nfds;
		fd_set readfds = masterfds;
		fd_set writefds;

		FD_ZERO(&writefds);
		if (ibuf.w.queued > 0)
			FD_SET(ibuf.fd, &writefds);

		if ((nfds = select(fd_max + 1, &readfds, &writefds, NULL, NULL)) == -1)
			log_err(1, "[priv] select");

		/* Flush our pending imsgs */
		if (nfds > 0 && FD_ISSET(ibuf.fd, &writefds))
			if (msgbuf_write(&ibuf.w) < 0) {
				log_warnx("[priv] pipe write error");
				quit = 1;
			}

		/* Read what Martine is asking to Martin  */
		if (nfds > 0 && FD_ISSET(ibuf.fd, &readfds)) {
			--nfds;
			if (dispatch_imsg(&ibuf) == -1)	
				quit = 1;
		}

		if (sigchld == 1) {
			quit = 1;
			chld_pid = 0;
			sigchld = 0;
		}
	}

	signal(SIGCHLD, SIG_DFL);

	if (chld_pid != 0)
		kill(chld_pid, SIGTERM);

	msgbuf_clear(&ibuf.w);
	log_info("[priv] tnetacle exiting");
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
	int n;
	ssize_t datalen;
	char buf[128];

	n = imsg_read(ibuf);
	if (n == -1) {
		log_warnx("[priv] loose some imsgs");
		imsg_clear(ibuf);
		return 0;
	}

	if (n == 0) {
		log_warnx("[priv] pipe closed");
		return -1;
	}

	for (;;) {
		/* Loops through the queue created by imsg_read */
		n = imsg_get(ibuf, &imsg);
		if (n == -1) {
			log_warnx("[priv] imsg_get");
			return -1;
		}

		/* Nothing was ready, return to the main loop */
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_CREATE_DEV:
			dev = tnt_tun_open();
			imsg_compose(ibuf, IMSG_CREATE_DEV, 0, 0, -1,
			    &(dev->fd), sizeof(int));
			break;
		case IMSG_SET_IP:
			if (dev == NULL) {
				log_warnx("[priv] can't set ip, use IMSG_CREATE_DEV first");
				break;
			}
			datalen = imsg.hdr.len - IMSG_HEADER_SIZE;
			(void)memset(buf, '\0', sizeof buf);
			(void)memcpy(buf, imsg.data, sizeof buf);
			buf[datalen] = '\0';
			
			log_info("[priv] receive IMSG_SET_IP: %s", buf);
			tnt_tun_set_ip(dev, buf);
			break;
		default:
			break;
		}
		imsg_free(&imsg);
	}
	return 0;
}

