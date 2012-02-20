/* Here will be the Windows NT logging API implementation */

#include <stdio.h>
#include "log.h"

void
log_init(void) {
}

void
log_err(int eval, const char *str, ...) {
	printf("%s\n", str);
}

void
log_errx(int eval, const char *str) {
	printf("%s\n", str);
}

void
log_warn(const char *str, ...) {
	printf("%s\n", str);
}

void
log_warnx(const char *str, ...) {
	printf("%s\n", str);
}

void
log_notice(const char *str, ...) {
	printf(str);
}

void
log_info(const char *str, ...) {
	printf("%s\n", str);
}

void
log_debug(const char *str, ...) {
	printf("%s\n", str);
}

