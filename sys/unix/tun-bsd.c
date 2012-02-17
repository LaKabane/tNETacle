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
#include <sys/param.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_tun.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "tun.h"

int
tnt_tun_set_ip(struct device *dev, const char *addr) {
    struct sockaddr_in sai;
    int sock = -1;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	log_warn("[priv] %s: configuration socket", __func__);
	return -1;
    }

    memset(&sai, '\0', sizeof sai);
    sai.sin_family = AF_INET;
    sai.sin_port = 0;
    sai.sin_addr.s_addr = inet_addr(addr);
    memcpy(&(dev->ifr.ifr_addr), &sai, sizeof(struct sockaddr));

    if (ioctl(sock, SIOCSIFADDR, &(dev->ifr)) == -1) {
	log_warn("[priv] %s: ioctl SIOCSIFADDR", __func__);
	return -1;
    }

    close(sock);
    log_info("[priv] set %s ip to %s", dev->ifr.ifr_name, addr);
    return 0;
}

int
tnt_tun_set_netmask(struct device *dev, const char *netmask) {
    struct sockaddr_in sai;
    int sock = -1;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	log_warn("[priv] %s: configuration socket", __func__);
	return -1;
    }

    memset(&sai, '\0', sizeof sai);
    sai.sin_family = AF_INET;
    sai.sin_port = 0;
    sai.sin_addr.s_addr = inet_addr(netmask);
    memcpy(&(dev->ifr.ifr_addr), &sai, sizeof(struct sockaddr));

    if (ioctl(sock, SIOCSIFNETMASK, &(dev->ifr)) == -1) {
	log_warn("[priv] %s: ioctl SIOCSIFNETMASK", __func__);
	return -1;
    }

    close(sock);
    log_info("[priv] set %s netmask to %s", dev->ifr.ifr_name, netmask);
    return 0;
}

int
tnt_tun_set_mac(struct device *dev, const char *mac) {
#ifdef OpenBSD
    log_info("tnt_tun_set_mac is currently not supported on your system");
    return -1;
#else
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
#endif
}

struct device *
tnt_tun_open(void) {
    struct device *dev = NULL;
    int fd = -1;
    int sock = -1;
    int tun = 0;
    char path[MAXPATHLEN];

    /* Open the tun interface */
    for (; tun < 256; ++tun) { /* XXX: magic value */
	(void)memset(path, '\0', sizeof path);
	(void)snprintf(path, sizeof path, "%s%i", "/dev/tun", tun);
	if ((fd = open(path, O_RDWR)) > 0)
	    break;
	else
	    log_debug("open tun%i", tun);
    }
    if (fd == -1 || fd == 256) {
	log_warn("open failed: %s", path);
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

    /* Get the internal parameters of ifr */
    snprintf(dev->ifr.ifr_name, sizeof(dev->ifr.ifr_name),
      "tun%i", tun);
    if (ioctl(sock, SIOCGIFFLAGS, &(dev->ifr)) == -1) {
	log_warn("ioctl SIOCGIFFLAGS");
	goto clean;
    }

    /* Bring the interface up */
    dev->ifr.ifr_flags |= IFF_UP;
    if (ioctl(sock, SIOCSIFFLAGS, &(dev->ifr)) == -1) {
	log_warn("ioctl SIOCSIFFLAGS");
	goto clean;
    }

    dev->fd = fd;

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
    (void)close(dev->fd);
}

