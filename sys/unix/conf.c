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
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Original copyright notice :
 * David Leonard <d@openbsd.org>, 1999. Public domain.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>

#include <dirent.h> /* For MAXNAMLEN */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "tnetacle.h"

/*
 * mmap the given config file and start the parsing
 */
static void
load_config(int fd, unsigned long int len) {
	void *buf;
	char *p;

	buf = mmap(0, len, PROT_READ, MAP_PRIVATE | MAP_FILE, fd, 0);
	if (buf == MAP_FAILED)
		perror("mmap");
		return;
	}

	p = (char *)buf;
	while (p != NULL && *p != '\0') {
		p = tnt_parse_line(p);
	}

	if (munmap(buf, len) == -1)
		perror("munmap");
}

/*
 * load various config file, allowing later ones to 
 * overwrite earlier values
 */
void
tnt_conf(void) {
	char *home;
	char nm[MAXNAMLEN + 1];
	char *fnms[] = { 
		"/etc/tNETacle.conf",
		"%s/.tNETacle.conf",
		".tNETacle.conf",
		NULL
	};
	int fn;
	int fd;
	struct stat st;

	/* All the %s's get converted to $HOME */
	if ((home = getenv("HOME")) == NULL)
		home = "";

	for (fn = 0; fnms[fn]; fn++) {
		(void)snprintf(nm, sizeof nm, fnms[fn], home);
		if ((fd = open(nm, O_RDONLY)) != -1) {
			if (stat(nm, &st) == -1)
				perror(nm);

			load_config(fd, st.st_size);
			(void)close(fd);
		} 
		else if (errno != ENOENT)
			(void)fprintf(stderr, "config: %s", nm);
	}
}

