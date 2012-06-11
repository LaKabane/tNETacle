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
#if !defined Windows
#include <unistd.h>
#endif

#include "tun.h"

/* Will search the first available tap device with both libraries */
struct device *
tnt_ttc_open(void) {
	struct device *dev;

#if defined USE_LIBTUNTAP
	if ((dev = tnt_tt_init()) == NULL)
		return NULL;

	if (tnt_tt_start(dev, TNT_TUNMODE_ETHERNET, TNT_TUNID_ANY) == -1) {
		tnt_tt_release(dev);
		return NULL;
	}
#elif defined USE_TAPCFG
	dev = tapcfg_init();
	if (tapcfg_start(dev, NULL, 1) == -1) {
		tapcfg_destroy(dev);
		return NULL;
	}
#endif
	return dev;
}

void
tnt_ttc_close(struct device *dev) {
#if defined USE_LIBTUNTAP
	tnt_tt_destroy(dev);
#elif defined USE_TAPCFG
	tapcfg_destroy(dev);
#endif
}

int
tnt_ttc_set_ip(struct device *dev, const char *addr) {
	char *ip, *mask;
	short netbits;

	ip = strdup(addr);
	if (ip == NULL)
		return -1;

	mask = strchr(ip, '/');
	if (mask == NULL)
		return -1;

#if defined USE_LIBTUNTAP
	*mask= '\0';
	++mask;
	tnt_tt_set_ip(dev, ip, mask);
	(void)netbits;
#elif defined USE_TAPCFG
	netbits = (short)evutil_strtoll(mask + 1, NULL, 10);
	if (netbits >= 1 && netbits <= 32) {
		*mask= '\0';
		tapcfg_iface_set_ipv4(dev, ip, netbits);
	}
#endif
	free(ip);
	return 0;
}

int
tnt_ttc_up(struct device *dev) {
#if defined USE_LIBTUNTAP
	return tnt_tt_up(dev);
#elif defined USE_TAPCFG
	return tapcfg_iface_set_status(dev, TAPCFG_STATUS_ALL_UP);
#endif
}

int
tnt_ttc_down(struct device *dev) {
#if defined USE_LIBTUNTAP
	return tnt_tt_down(dev);
#elif defined USE_TAPCFG
	return tapcfg_iface_set_status(dev, TAPCFG_STATUS_ALL_DOWN);
#endif
}

int
tnt_ttc_get_fd(struct device *dev) {
#if defined USE_LIBTUNTAP
	return TNT_TT_GET_FD(dev);
#elif defined USE_TAPCFG
	return tapcfg_get_fd(dev);
#endif
}
