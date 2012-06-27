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

#if defined Unix
# include <pwd.h>
#endif

struct passwd;

#ifndef TNETACLE_H__
#define TNETACLE_H__

/*
 * Add this user to your system, with daemon class,
 * nologin shell and /var/empty for home.
 */
#define TNETACLE_USER "_tnetacle"

#define TNETACLE_DEFAULT_PORT	4242
#define TNETACLE_MAX_PORTS	256

/*
 * Definition of types for our imsg.
 */
enum imsg_type {
	IMSG_NONE,
	IMSG_CREATE_DEV,
	IMSG_SET_IP,
};

char 		*tnt_getprogname(void);
void    	 tnt_setproctitle(const char *);
int		 tnt_fork(int [2]);
int		 tnt_daemonize(void);

/* src/conf.c */
int	 tnt_parse_buf(char *, size_t);
/* and sys/ specific call */
int	 tnt_parse_file(const char *);

#endif

