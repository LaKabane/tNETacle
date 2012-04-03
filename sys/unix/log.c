/*	$OpenBSD: log.c,v 1.8 2007/08/22 21:04:30 ckuethe Exp $ */

/*
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include "tnetacle.h"
#include "log.h"

extern int conf_debug;
static char *prefix = "";

static void
vlog(int pri, const char *fmt, va_list ap) {
	char	*nfmt;

	if (conf_debug == 1) {
		/* best effort in out of mem situations */
		if (asprintf(&nfmt, "[%s] %s\n", prefix, fmt) == -1) {
			vfprintf(stderr, fmt, ap);
			fprintf(stderr, "\n");
		} else {
			vfprintf(stderr, nfmt, ap);
			free(nfmt);
		}
		fflush(stderr);
	} else {
		if (asprintf(&nfmt, "[%s] %s\n", prefix, fmt) == -1)
			vsyslog(pri, fmt, ap);
		else {
			vsyslog(pri, nfmt, ap);
			free(nfmt);
		}
	}
}

static void
flog(int pri, const char *fmt, ...) {
	va_list	ap;

	va_start(ap, fmt);
	vlog(pri, fmt, ap);
	va_end(ap);
}

void
log_set_prefix(char *s) {
	prefix = s;
}

void
log_init(void) {
	char	*progname = tnt_getprogname();

	if (conf_debug == 0)
		openlog(progname, LOG_PID | LOG_NDELAY, LOG_USER);

	tzset();
}

void
log_err(int eval, const char *emsg, ...) {
	if (emsg == NULL)
		flog(LOG_ERR, "error: %s", strerror(errno));
	else
		if (errno)
			flog(LOG_ERR, "error: %s: %s",
			    emsg, strerror(errno));
		else
			flog(LOG_ERR, "error: %s", emsg);

	exit(eval);
}

void
log_errx(int eval, const char *emsg) {
	errno = 0;
	log_err(eval, emsg);
}

void
log_warn(const char *emsg, ...) {
	char	*nfmt;
	va_list	 ap;

	/* best effort to even work in out of memory situations */
	if (emsg == NULL)
		flog(LOG_WARNING, "%s", strerror(errno));
	else {
		va_start(ap, emsg);

		if (asprintf(&nfmt, "%s: %s", emsg, strerror(errno)) == -1) {
			/* we tried it... */
			vlog(LOG_WARNING, emsg, ap);
			flog(LOG_ERR, "%s", strerror(errno));
		} else {
			vlog(LOG_WARNING, nfmt, ap);
			free(nfmt);
		}
		va_end(ap);
	}
}

void
log_warnx(const char *emsg, ...) {
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_WARNING, emsg, ap);
	va_end(ap);
}

void
log_notice(const char *emsg, ...) {
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_NOTICE, emsg, ap);
	va_end(ap);
}
void
log_info(const char *emsg, ...) {
	va_list	 ap;

	va_start(ap, emsg);
	vlog(LOG_INFO, emsg, ap);
	va_end(ap);
}

void
log_debug(const char *emsg, ...) {
	va_list	 ap;

	if (conf_debug == 1) {
		va_start(ap, emsg);
		vlog(LOG_DEBUG, emsg, ap);
		va_end(ap);
	}
}

