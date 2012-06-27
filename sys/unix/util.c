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
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef HAVE_BSD_COMPAT
# include <bsd/unistd.h>
#endif
#include "tnetacle.h"
#include "log.h"

void
tnt_setproctitle(const char *s) {
#ifdef HAVE_SETPROCTITLE
    setproctitle(s);
#else
    log_info("your system does not have setproctitle syscall");
    (void)s;
#endif
}

char *
tnt_getprogname(void) {
#if defined OpenBSD || Minix
    extern char *__progname;
    return __progname;
#elif defined NetBSD || FreeBSD || Darwin
    return (char *)getprogname();
#elif defined Linux
    extern char *program_invocation_short_name;
    return program_invocation_short_name;
#else
    return "tNETacle";
#endif
}

/* Come from the BSD daemon() */
int
tnt_daemonize(void) {
    int fd;
    
    switch (fork()) {
    case -1:
        return -1;
    case 0:
        break;
    default:
        exit(0);
    }
    
    if (setsid() == -1)
        return -1;
    
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        (void)dup2(fd, STDIN_FILENO);
        (void)dup2(fd, STDOUT_FILENO);
        (void)dup2(fd, STDERR_FILENO);
        if (fd > 2)
            (void)close(fd);
    }
    return 0;
}

