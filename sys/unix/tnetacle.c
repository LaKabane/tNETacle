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

volatile sig_atomic_t chld_quit;
int tnt_dispatch_imsg(struct imsgbuf *ibuf);

/* XXX: Do something better when Mota will import his network code */
int tun_fd = -1;
fd_set masterfds;

void
chld_sighdlr(int sig) {
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		chld_quit = 1;
		break;
	}
}

void
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
		log_err(1, "[unpriv] can't drop privileges");
#ifdef _POSIX_SAVED_IDS
	if (setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) == -1 ||
	  setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid) == -1)
		log_err(1, "[unpriv] can't drop privileges");
#else
	/* Fallback to setuid, but it might not work properly */
	if (setuid(pw->pw_uid) == -1 || setgid(pw->pw_gid) == -1)
		log_err(1, "[unpriv] can't drop privileges");
#endif
}

int
tnt_fork(int imsg_fds[2], struct passwd *pw) {
	pid_t pid;
	struct imsgbuf ibuf;
	/* XXX: To remove when Mota will bring his network code */
	int fd_max;

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

	signal(SIGTERM, chld_sighdlr);
	signal(SIGINT, chld_sighdlr);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGCHLD, SIG_DFL);

	if (close(imsg_fds[0]))
		log_notice("[unpriv] close");

	imsg_init(&ibuf, imsg_fds[1]);

	log_info("[unpriv] tnetacle ready");

	fd_max = ibuf.fd;
	FD_ZERO(&masterfds);
	FD_SET(ibuf.fd, &masterfds);

	/* Immediately request the creation of a tun interface */
	imsg_compose(&ibuf, IMSG_CREATE_DEV, 0, 0, -1, NULL, 0);

	while (chld_quit == 0) {
		int nfds;
		fd_set readfds = masterfds;
		fd_set writefds;

		FD_ZERO(&writefds);
		if (ibuf.w.queued > 0)
			FD_SET(ibuf.fd, &writefds);

		if ((nfds = select(fd_max + 1, &readfds, &writefds, NULL, NULL)) == -1)
			log_err(1, "[unpriv] select");

		if (nfds > 0 && FD_ISSET(ibuf.fd, &writefds)) {
			if (msgbuf_write(&ibuf.w) < 0) {
				log_warnx("[unpriv] pipe write error");
				chld_quit = 1;
			}
		}

		/* Read what Martin replied to Martine */
		if (nfds > 0 && FD_ISSET(ibuf.fd, &readfds)) {
			--nfds;
			if (tnt_dispatch_imsg(&ibuf) == -1)
				chld_quit = 1;
		}
	}
	/* cleanely exit */
	msgbuf_write(&ibuf.w);
	msgbuf_clear(&ibuf.w);
	
	log_info("[unpriv] tnetacle exiting");
	exit(TNT_OK);
}

/*
 * The purpose of this function is to handle requests sent by the
 * root level process.
 */
int
tnt_dispatch_imsg(struct imsgbuf *ibuf) {
	struct imsg imsg;
	int n;

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

	for (;;) {
		/* Loops through the queue created by imsg_read */
		n = imsg_get(ibuf, &imsg);
		if (n == -1) {
			log_warnx("[unpriv] imsg_get");
			return -1;
		}

		/* Nothing was ready */
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_CREATE_DEV:
			if (imsg.hdr.len != IMSG_HEADER_SIZE + sizeof(tun_fd))
				log_errx(1, "[unpriv] invalid IMSG_CREATE_DEV received");
			(void)memcpy(&tun_fd, imsg.data, sizeof tun_fd);
			log_info("[unpriv] receive IMSG_CREATE_DEV: fd %i", tun_fd);
			/* directly ask to configure the tun device */
			FD_SET(tun_fd, &masterfds);

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
	return 0;
}

