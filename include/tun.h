/*
 * Copyright (c) 2012 Tristan Le Guern <leguern AT medu DOT se>
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

#ifndef TNETACLE_TUN_H_
#define TNETACLE_TUN_H_

# if defined USE_LIBTUNTAP
#  include <tuntap.h>
# elif defined USE_TAPCFG
#  include <tapcfg.h>
#  define device tapcfg_t
# else
#  error "You must define USE_LIBTUNTAP or USE_TAPCFG"
# endif

struct device	*tnt_ttc_open(int);
void		 tnt_ttc_close(struct device *);
int		 tnt_ttc_set_ip(struct device *, const char *);
int		 tnt_ttc_up(struct device *);
int		 tnt_ttc_down(struct device *);
int		 tnt_ttc_get_fd(struct device *);

#endif

