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
#include "tun.h"
#include "log.h"

/* Will search the first available tap device with both libraries */
struct device *
tnt_ttc_open(int tunmode) {
	struct device *dev;

#if defined USE_LIBTUNTAP
	if ((dev = tuntap_init()) == NULL)
		return NULL;

    if (tunmode == TNT_TUNMODE_TUNNEL)
        tunmode = TUNTAP_TUNMODE_TUNNEL;
    else if (tunmode == TNT_TUNMODE_ETHERNET)
        tunmode = TUNTAP_TUNMODE_ETHERNET;
    else
        return NULL;

	if (tuntap_start(dev, tunmode, TUNTAP_TUNID_ANY) == -1) {
		tuntap_release(dev);
		return NULL;
	}
#elif defined USE_TAPCFG
    if (tunmode == TNT_TUNMODE_TUNNEL) {
        log_errx(1, "Layer 3 tunnelling is not implemented for your system "
          "(because of tapcfg)");
        /* NOTREACHED */
    }

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
	tuntap_destroy(dev);
#elif defined USE_TAPCFG
	tapcfg_destroy(dev);
#endif
}

int
tnt_ttc_set_ip(struct device *dev, const char *addr) {
	char *ip, *mask;
	int netbits;
	int ret;

	ret = 0;
	ip = strdup(addr);
	if (ip == NULL)
		return -1;

	mask = strchr(ip, '/');
	if (mask == NULL)
		return -1;
	*mask= '\0';
	++mask;

	netbits = (short)evutil_strtoll(mask, NULL, 10);
#if defined USE_LIBTUNTAP
	ret = tuntap_set_ip(dev, ip, netbits);
#elif defined USE_TAPCFG
	ret = tapcfg_iface_set_ipv4(dev, ip, netbits);
#endif
	free(ip);
	return ret;
}

int
tnt_ttc_up(struct device *dev) {
#if defined USE_LIBTUNTAP
	return tuntap_up(dev);
#elif defined USE_TAPCFG
	return tapcfg_iface_set_status(dev, TAPCFG_STATUS_ALL_UP);
#endif
}

int
tnt_ttc_down(struct device *dev) {
#if defined USE_LIBTUNTAP
	return tuntap_down(dev);
#elif defined USE_TAPCFG
	return tapcfg_iface_set_status(dev, TAPCFG_STATUS_ALL_DOWN);
#endif
}

intptr_t
tnt_ttc_get_fd(struct device *dev) {
#if defined USE_LIBTUNTAP
	return TUNTAP_GET_FD(dev);
#elif defined USE_TAPCFG
	return tapcfg_get_fd(dev);
#endif
}

