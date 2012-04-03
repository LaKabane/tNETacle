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

#include <Winbase.h> 

#include "tnetacle.h"
#include "log.h"

/*
 * MapViewOfFile the given config file and start the parsing
 */
static void
load_config(HANDLE hdl) {
	void *buf;
	char *p;

	buf = MapViewOfFile(hdl, FILE_MAP_READ, 0, 0, 0);
	if (buf == NULL)
		log_err(1, "MapViewOfFile");

	p = (char *)buf;
	while (p != NULL && *p != '\0') {
		p = tnt_parse_line(p);
	}

	if (UnMapViewOfFile(buf) == 0)
		log_warn("UnMapViewOfFile");
}

/*
 * load various config file, allowing later ones to 
 * overwrite earlier values
 */
void
config(void) {
	char nm[MAXNAMLEN + 1];
	/* TODO: Get normal path to store config file */
	char *fnms[] = { 
		"...",
		NULL
	};
	int fn;
	HANDLE hdl;
	struct stat st;

	for (fn = 0; fnms[fn]; fn++) {
		(void)snprintf(nm, sizeof nm, fnms[fn], home);
		if ((hdl = OpenFileMapping(FILE_MAP_READ, FALSE, nm)) != -1) {
			load_config(hdl);
			(void)CloseHandle(hdl);
		} 
		else if (errno != ENOENT)
			log_notice("config: %s", nm);
	}
}

