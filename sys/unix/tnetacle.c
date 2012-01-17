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
#include <sys/stat.h>

#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tntexits.h"
#include "tnetacle.h"
#include "log.h"

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
	struct imsg imsg;
	struct imsgbuf ibuf;

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

	/*signal(SIGTERM, unpriv_sighdlr);
	signal(SIGINT, unpriv_sighdlr);*/
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGCHLD, SIG_DFL);

	if (close(imsg_fds[0]))
		log_notice("[unpriv] close");

	imsg_init(&ibuf, imsg_fds[1]);

	log_info("tnetacle unpriv ready");

	for (;;) {
		n = imsg_read(&ibuf);
		if (n == -1) {
			log_warnx(1, "[unpriv] loose some imsgs");
			imsg_clear(&ibuf);
			continue;
		}
		if (n == 0)
			log_errx(1, "[unpriv] pipe closed");

		for (;;) {
			/* Loops through the queue created by imsg_read */
			n = imsg_get(&ibuf, &imsg);
			if (n == -1)
				log_err(1, "[unpriv] get");

			/* Nothing was ready */
			if (n == 0)
				break;

			switch (imsg.hdr.type) {
			default:
				break;
			}
			imsg_free(&imsg);
		}
	}
	exit(TNT_OK);
}

