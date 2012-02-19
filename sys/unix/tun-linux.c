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

struct device {
	int		af;	/* AF_INET || AF_INET6 */
	int		fd;
	struct ifreq	ifr;
};

static int
tnt_tun_set_ip(int sock, struct device *dev) {
	struct sockaddr_in sai;

	memset(&sai, '\0', sizeof sai);
	sai.sin_family = AF_INET;
	sai.sin_port = 0;
	sai.sin_addr.s_addr = inet_addr("192.168.0.42");
	memcpy(&(dev->ifr.ifr_addr), &sai, sizeof(struct sockaddr));

	if (ioctl(sock, SIOCSIFADDR, &(dev->ifr)) == -1) {
	    log_warn("ioctl SIOCSIFADDR");
	    return -1;
	}
	return 0;
}

struct device *
tnt_tun_open(void) {
	struct device *dev = NULL;
	int fd = -1;
	int sock = -1;

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

	if (tnt_tun_set_ip(sock, dev) == -1)
		goto clean;

	/* Get the internal parameters of ifr */
	if (ioctl(sock, SIOCGIFFLAGS, &(dev->ifr)) == -1) {
		log_warn("ioctl SIOCGIFFLAGS");
		goto clean;
	}

	dev->ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
	if (ioctl(sock, SIOCSIFFLAGS, &(dev->ifr)) == -1) {
		log_warn("ioctl SIOCSIFFLAGS");
		goto clean;
	}

	log_info("tunnel fd %d (%s)\n", dev->fd, dev->ifr.ifr_name);

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
	close(dev->fd);
    	/* And other stuff later ? */
}

/*
TODO:
tnt_tun_read();
tnt_tun_write();
*/

/*
HOW TO USE:
int
main(void) {
	struct device *dev;

	dev = tnt_tun_open();
	...
	tnt_tun_close(dev);
	return 0;
}
*/
