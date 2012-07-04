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

#ifdef WIN32
#include <winsock2.h>
#include <Windows.h>
#define _PATH_DEFAULT_CONFIG_FILE "./"
#define __func__ __FUNCTION__
#endif

#include <stdio.h>

#include "tnetacle.h"
#include "log.h"

/*
 * Load the configuration file given in argument.
 * If file is NULL, tnt_parse_config_file() will load the default
 * configuration file.
 */
int
tnt_parse_file(const char *file) {
    char buf[65000];
	int n;
    HANDLE hdl;

    if (file == NULL) {
        file = _PATH_DEFAULT_CONFIG_FILE;
    }

	hdl = CreateFile(TEXT(file), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hdl == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "%s: Can't open\n", file);
	}

	while (ReadFile(hdl, buf, sizeof buf, &n, NULL) == TRUE) {
		int ret;
		fprintf(stderr, "ByteRead: %i\n", n);
		if (n == 0)
			break;
		ret = tnt_parse_buf((char *)buf, n);
        if (ret == -1) {
            perror("tnt_parse_buf");
            break; /* XXX: ???*/
        }
	}

	(void)CloseHandle(hdl);
    return 0;
}

