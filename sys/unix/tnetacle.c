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

volatile sig_atomic_t chld_quit;

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

	/*imsg_init(&ibuf, imsg_fds[1]);*/

	log_info("tnetacle unpriv ready");

	while (chld_quit == 0) {
		/* This is where Mota will put his network code */
		(void)select(1, NULL, NULL, NULL, NULL);

		/*
		 * If there are pending actions from [priv],
		 * call tnt_dispatch_imsg()
		 */
	}
	exit(TNT_OK);
}

/*
 * The purpose of this function is to handle requests sent by the
 * root level process.
 * If nothing is to be received, do not compile it.
 */
#if 0
int
tnt_dispatch_imsg(void) {
	struct imsg imsg;
	struct imsgbuf ibuf;
	int n;

	n = imsg_read(&ibuf);
	if (n == -1) {
		log_warnx(1, "[unpriv] loose some imsgs");
		imsg_clear(&ibuf);
		return 0;
	}

	if (n == 0) {
		log_warnx(1, "[unpriv] pipe closed");
		return -1;
	}

	for (;;) {
		/* Loops through the queue created by imsg_read */
		n = imsg_get(&ibuf, &imsg);
		if (n == -1) {
			log_warnx(1, "[unpriv] imsg_get");
		}

		/* Nothing was ready */
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		default:
			break;
		}
		imsg_free(&imsg);
	}
	return 0;
}
#endif

