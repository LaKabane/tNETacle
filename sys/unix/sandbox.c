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
#include <sys/time.h>
#include <sys/resource.h>

#include "log.h"

/*
 * Prevent the tNETacle to open files and to spawn other process.
 */
void
sandbox(void) {
    struct rlimit rl;

    rl.rlim_cur = rl.rlim_max = 0;
    if (setrlimit(RLIMIT_FSIZE, &rl) == -1)
	log_err(1, "setrlimit RLIMIT_FSIZE");
    if (setrlimit(RLIMIT_NOFILE, &rl) == -1)
	log_err(1, "setrlimit RLIMIT_NOFILE");
    if (setrlimit(RLIMIT_NPROC, &rl) == -1)
	log_err(1, "setrlimit RLIMIT_NPROC");
}

