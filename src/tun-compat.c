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

/*
 * Functions from this file are prefixed with the namespace
 * ttc_, for "tNETacle tun compat".
 *
 * Part of this work come from Nizox's virtual hub project:
 *    - https://github.com/nizox/hub
 */

#include <stdlib.h>
#include <string.h>
#if defined Unix
# include <unistd.h>
#endif

#include <event2/util.h>

#include "tnetacle.h"
#include "options.h"
#include "log.h"
#include "tun.h"

int
tnt_ttc_set_ip(struct device *dev, const char *addr) {
	char *ip, *mask;
	int netbits;
	int ret;

	log_info(addr);
	ret = 0;
	ip = strdup(addr);
	if (ip == NULL)
		return -1;

	mask = strchr(ip, '/');
	if (mask == NULL)
    {
        free(ip);
		return -1;
    }
	*mask= '\0';
	++mask;

	netbits = (short)evutil_strtoll(mask, NULL, 10);
	ret = tuntap_set_ip(dev, ip, netbits);
	free(ip);
	return ret;
}

int
tnt_ttc_get_mtu(struct device *dev) {
	return tuntap_get_mtu(dev);
}

