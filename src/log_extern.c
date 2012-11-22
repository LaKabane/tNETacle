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

#include <event2/event.h>
#include <tuntap.h>

#include "tnetacle.h"
#include "log.h"

extern int debug;

void
tnet_libevent_log(int severity, const char *msg) {
    switch (severity) {
    case EVENT_LOG_DEBUG:
        if (debug == 1) {
            log_debug("[libevent]%s", msg);
        }
        break;
    case EVENT_LOG_MSG:
        log_info("[libevent] %s", msg);
        break;
    case EVENT_LOG_WARN:
        log_warnx("[libevent] %s", msg);
        break;
    case EVENT_LOG_ERR:
        /* We will not quit */
        log_warnx("[libevent] ERROR: %s", msg);
        break;
    default:
        log_notice("Received strange log from libevent");
        break;
    }
}

void
tnet_libtuntap_log(int severity, const char *msg) {
    switch (severity) {
    case TUNTAP_LOG_DEBUG:
        if (debug == 1) {
            log_debug("[libtuntap]%s", msg);
        }
        break;
    case TUNTAP_LOG_NOTICE:
        log_notice("[libtuntap] %s", msg);
        break;
    case TUNTAP_LOG_INFO:
        log_info("[libtuntap] %s", msg);
        break;
    case TUNTAP_LOG_WARN:
        log_warnx("[libtuntap] %s", msg);
        break;
    case TUNTAP_LOG_ERR:
        /* We will not quit */
        log_warnx("[libtuntap] ERROR: %s", msg);
        break;
    default:
        log_notice("Received strange log from libtuntap");
        break;
    }
}
