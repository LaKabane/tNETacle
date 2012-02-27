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

struct device {
	int		af;	/* AF_INET || AF_INET6 */
	int		fd;
	struct ifreq	ifr;
#ifdef HAVE_NETLINK
  struct nl_sock *sk;
  struct rtnl_link *link;
  struct nl_cache *cache;
  struct rtnl_addr *addr;
#endif
};

struct device	*tnt_tun_open(void);
void		 tnt_tun_close(struct device *);
int		 tnt_tun_set_ip(struct device *, const char *);
int		 tnt_tun_set_netmask(struct device *, const char *);
int		 tnt_tun_set_mac(struct device *, const char *);

#endif

