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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tnetacle.h"
#include "tntexits.h"

int debug;
volatile sig_atomic_t sigchld;
volatile sig_atomic_t quit;

static void usage(void);

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

/* 
 * Added here for initial convenience, but the main will
 * move under sys/unix later.
 */
int
main(int argc, char *argv[]) {
	int ch;
	pid_t chld_pid;
	struct passwd *pw;
	int imsg_fds[2];
	struct imsg imsg;
	struct imsgbuf ibuf;

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

	if (geteuid())
		log_errx(1, "need root privileges");

	if ((pw = getpwnam(TNETACLE_USER)) == NULL)
		log_errx(TNT_NOUSER, "unknown user %s", TNETACLE_USER);

	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, imsg_fds) == -1)
		log_err(1, "socketpair");

	signal(SIGCHLD, sighdlr);
	chld_pid = tnt_fork(imsg_fds, pw);

	signal(SIGTERM, sighdlr);
	signal(SIGINT, sighdlr);

	if (close(imsg_fds[1]))
		log_notice("[priv] close");

	imsg_init(&ibuf, imsg_fds[1]);

	while (quit == 0)
		int n = imsg_read(&ibuf);
		if (n == -1) {
			log_warnx(1, "[priv] loose some imsgs");
			imsg_clear(&ibuf);
			continue;
		}
		if (n == 0)
			log_errx(1, "[priv] pipe closed");

		for (;;) {
			/* Loops through the queue created by imsg_read */
			n = imsg_get(&ibuf, &imsg);
			if (n == -1)
				log_err(1, "[priv] get");

			/* Nothing was ready */
			if (n == 0)
				break;

			switch (imsg.hdr.type) {
			default:
				break;
			}
			imsg_free(&imsg);
		}
		
		if (sigchld == 1) {
			quit = 1;
			chld_pid = 0;
			sigchld = 0;
		}
	}

	signal(SIGCHLD, SIG_DFL);

	if (chld_pid != 0)
		kill(chld_pid, SIGTERM)

	msgbuf_clear(*ibuf->w);
	log_info("Terminating");
	return TNT_OK;
}

static void
usage(void) {
	char *progname = tnt_getprogname();

	(void)fprintf(stderr, "%s: [-dh]\n", progname);
	exit(TNT_USAGE);
}

