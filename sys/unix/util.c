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

#include <stdlib.h>

#include "tnetacle.h"
#include "log.h"

void
tnt_setproctitle(const char *s) {
#ifdef HAVE_SETPROCTITLE
	setproctitle(s);
#else
	log_info("Your system does not have setproctitle syscall");
  (void)s;
#endif
}

char *
tnt_getprogname(void) {
#if defined OpenBSD || Minix
	extern char *__progname;
	return __progname;
#elif defined NetBSD || FreeBSD || Darwin
	return getprogname();
#elif defined Linux
	extern char *program_invocation_short_name;
	return program_invocation_short_name;
#else
	return "tNETacle";
#endif
}

