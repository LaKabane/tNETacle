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
 * ttc_, for tNETacle tun compat.
 */

#include <unistd.h>

#if defined USE_LIBTUNTAP
	/* Lucky you ! */
#elif defined USE_TAPCFG
#	define device tapcfg_t
#else
# error "You must define USE_LIBTUNTAP or USE_TAPCFG"
#endif

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

int
tnt_ttc_set_ip(struct device *dev, const char *ip, const char *mask) {
#if defined USE_LIBTUNTAP
	return tnt_tt_set_ip(dev, ip, mask);
#elif defined USE_TAPCFG
	char buf[50];
	int bits;

	(void)memset(buf, '\0', sizeof buf);
	bits = inet_net_pton(AF_INET, mask, buf, sizeof buf);

	return tapcfg_iface_set_ipv4(dev, ip, bits);
#endif
}

