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
#include <stdarg.h>
#include <syslog.h>

#ifndef LOG_H__
#define LOG_H__

#define TNET_LOG_ERROR   LOG_EMERG /* Is generaly 0 */
#define TNET_LOG_WARNING LOG_WARNING
#define TNET_LOG_NOTICE  LOG_NOTICE
#define TNET_LOG_INFO    LOG_INFO
#define TNET_LOG_DEBUG   LOG_DEBUG

void		 log_init(void);
void		 log_set_prefix(char *);
void		 log_set_filter(int);
void		 log_err(int, const char *, ...);
void		 log_errx(int, const char *, ...);
void		 log_warn(const char *, ...);
void		 log_warnx(const char *, ...);
void		 log_notice(const char *, ...);
void		 log_info(const char *, ...);
void		 log_debug(const char *, ...);

#endif

