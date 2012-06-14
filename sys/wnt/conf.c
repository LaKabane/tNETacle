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
	int ret;
    void *buf;
    char *p;
    size_t size;
    HANDLE hdl;

    if (file == NULL) {
        file = _PATH_DEFAULT_CONFIG_FILE;
    }

    (void)GetFileSizeEx(hdl, (PLARGE_INTEGER)&size);

    if ((hdl = OpenFileMapping(FILE_MAP_READ, FALSE, file)) != NULL) {
        buf = MapViewOfFile(hdl, FILE_MAP_READ, 0, 0, 0);

        if (buf == NULL)
            log_err(1, "MapViewOfFile");

        ret = tnt_parse_buf((char *)buf, size);
        if (ret == -1) {
            perror("tnt_parse_buf");
            return -1;
        }

        if (UnmapViewOfFile(buf) == 0)
            log_warn("UnMapViewOfFile");
        (void)CloseHandle(hdl);
    } else {
        log_notice("config: %s", file);
        return -1;
    }
    return 0;
}

