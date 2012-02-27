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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tun.h"
#include "log.h"

#ifdef HAVE_NETLINK

#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>

int
tnt_tun_set_ip(struct device *dev, const char *addr) {
    struct nl_addr *local;

    nl_addr_parse(addr, AF_UNSPEC, &local);
    rtnl_addr_set_local(dev->addr, local);
    if (rtnl_addr_add(dev->sk, dev->addr, 0) == -1)
	return -1;
    return 0;
}

int
tnt_tun_set_netmask(struct device *dev, const char *netmask) {
    struct nl_addr *local;
    (void)netmask;

    local = rtnl_addr_get_local(dev->addr);
    rtnl_addr_delete(dev->sk, dev->addr, 0);
    rtnl_addr_set_local(dev->addr, local);
    rtnl_addr_set_prefixlen(dev->addr, 24);
    if (rtnl_addr_add(dev->sk, dev->addr, 0) == -1)
	return -1;
    return 0;
}

int
tnt_tun_set_mac(struct device *dev, const char *hwaddr) {
    struct nl_addr *hwlocal;

    if (nl_addr_parse(hwaddr, AF_UNSPEC, &hwlocal) == -1)
	return -1;
    rtnl_link_set_addr(dev->link, hwlocal);
    if (rtnl_link_add(dev->sk, dev->link, 0) == -1)
	return -1;
    return 0;
}

#else
int
tnt_tun_set_ip(struct device *dev, const char *addr) {
    struct sockaddr_in sai;
    int sock = -1;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	log_warn("%s: configuration socket", __func__);
	return -1;
    }

    memset(&sai, '\0', sizeof sai);
    sai.sin_family = AF_INET;
    sai.sin_port = 0;
    sai.sin_addr.s_addr = inet_addr(addr);
    memcpy(&(dev->ifr.ifr_addr), &sai, sizeof(struct sockaddr));

    if (ioctl(sock, SIOCSIFADDR, &(dev->ifr)) == -1) {
	log_warn("%s: ioctl SIOCSIFADDR", __func__);
	return -1;
    }

    close(sock);
    log_info("set %s ip to %s", dev->ifr.ifr_name, addr);
    return 0;
}

int
tnt_tun_set_netmask(struct device *dev, const char *netmask) {
    struct sockaddr_in sai;
    int sock = -1;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	log_warn("%s: configuration socket", __func__);
	return -1;
    }

    memset(&sai, '\0', sizeof sai);
    sai.sin_family = AF_INET;
    sai.sin_port = 0;
    sai.sin_addr.s_addr = inet_addr(netmask);
    memcpy(&(dev->ifr.ifr_addr), &sai, sizeof(struct sockaddr));

    if (ioctl(sock, SIOCSIFNETMASK, &(dev->ifr)) == -1) {
	log_warn("%s: ioctl SIOCSIFNETMASK", __func__);
	return -1;
    }

    close(sock);
    log_info("set %s netmask to %s", dev->ifr.ifr_name, netmask);
    return 0;
}

int
tnt_tun_set_mac(struct device *dev, const char *hwaddr) {
    int sock = -1;
    int i;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	log_warn("socket");
	return -1;
    }

    for (i = 0; i <= 6; ++i)
	dev->ifr.ifr_hwaddr.sa_data[i] = hwaddr[i];

    if (ioctl(sock, SIOCSIFHWADDR, &(dev->ifr)) == -1) {
	log_warn("ioctl SIOCSIFHWADDR");
	return -1;
    }

    log_debug("Set %s mac to %s", dev->ifr.ifr_name, hwaddr);
    return 0;
}
#endif
struct device *
tnt_tun_open(void) {
    struct device *dev = NULL;
    int fd = -1;
    int sock = -1;

    /* Open the tun interface */
    if ((fd = open("/dev/net/tun", O_RDWR)) == -1) {
	log_warn("open /dev/net/tun");
	goto clean;
    }

    if ((dev = malloc(sizeof(*dev))) == NULL) {
	log_warn("malloc (while allocating device)");
	goto clean;
    }

    /* configuration socket */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	log_warn("socket");
	goto clean;
    }

    (void)memset(&(dev->ifr), '\0', sizeof(dev->ifr));	

    dev->ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (ioctl(fd, TUNSETIFF, &(dev->ifr)) == -1) {
	log_warn("ioctl TUNSETIFF");
	goto clean;
    }

    /* Get the internal parameters of ifr */
    if (ioctl(sock, SIOCGIFFLAGS, &(dev->ifr)) == -1) {
	log_warn("ioctl SIOCGIFFLAGS");
	goto clean;
    }

    /* Bring the interface up */
    dev->ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(sock, SIOCSIFFLAGS, &(dev->ifr)) == -1) {
	log_warn("ioctl SIOCSIFFLAGS");
	goto clean;
    }

    dev->fd = fd;
#ifdef HAVE_NETLINK
    dev->sk = nl_socket_alloc();
    dev->addr = rtnl_addr_alloc();
    nl_connect(dev->sk, NETLINK_ROUTE);
    rtnl_link_alloc_cache(dev->sk, AF_UNSPEC, &dev->cache);
    dev->link = rtnl_link_get_by_name(dev->cache, "tun0"); /* Need the real name !*/
    rtnl_addr_set_ifindex(dev->addr, rtnl_link_get_ifindex(dev->link));
#endif

    close(sock);
    return dev;
clean:
    if (fd != -1)
	close(fd);
    if (sock != -1)
	close(sock);
    free(dev);
    return NULL;
}

void
tnt_tun_close(struct device *dev) {
#ifdef HAVE_NETLINK
    nl_close(dev->sk);
    nl_cache_free(dev->cache);
    rtnl_addr_put(dev->addr);
#endif
    close(dev->fd);
    /* And other stuff later ? */
}

