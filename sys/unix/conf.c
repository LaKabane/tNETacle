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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/param.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "pathnames.h"
#include "tnetacle.h"

/*
 * Load the configuration file given in argument.
 * If file is NULL, tnt_parse_file() will load the default
 * configuration file.
 */
int
tnt_parse_file(const char *file) {
    int fd;
    char *p;
    void *buf;
    struct stat st;

    if (file == NULL) {
        file = _PATH_DEFAULT_CONFIG_FILE;
    }

    if ((fd = open(file, O_RDONLY)) != -1) {
        if (stat(file, &st) == -1) {
	    (void)close(fd);
	    (void)fprintf(stderr, "%s: can't stat\n", file);
	    return -1;
	}

        if (st.st_mode & S_IRGRP || st.st_mode & S_IWGRP ||
	    st.st_mode & S_IXGRP || st.st_mode & S_IROTH ||
	    st.st_mode & S_IWOTH || st.st_mode & S_IXOTH) {
            (void)fprintf(stderr, "%s: insecure file permission\n", file);
	    (void)fchmod(fd, S_IRUSR|S_IWUSR);
        }

        buf = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE | MAP_FILE, fd, 0);
        if (buf == MAP_FAILED) {
            perror("mmap");
            return -1;
        }

        p = (char *)buf;
        while (p != NULL && *p != '\0') {
            p = tnt_parse_line(p);
        }

        if (munmap(buf, st.st_size) == -1)
            perror("munmap");

        (void)close(fd);
    } else if (errno != ENOENT) {
        (void)fprintf(stderr, "%s: can't open\n", file);
        return -1;
    }
    return 0;
}

