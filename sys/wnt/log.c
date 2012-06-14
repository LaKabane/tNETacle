/* Here will be the Windows NT logging API implementation */

#include <stdio.h>
#include "log.h"

void
log_init(void) {
}

void
log_err(int eval, const char *str, ...) {
	va_list	 ap;

	va_start(ap, str);
	vprintf(str, ap);
	puts("\n");
	va_end(ap);
}

void
log_errx(int eval, const char *str) {
	vprintf("%s\n", str);
}

void
log_warn(const char *str, ...) {
	va_list	 ap;

	va_start(ap, str);
	vprintf(str, ap);
	puts("");
	va_end(ap);
}

void
log_warnx(const char *str, ...) {
	va_list	 ap;

	va_start(ap, str);
	vprintf(str, ap);
	puts("");
	va_end(ap);
}

void
log_notice(const char *str, ...) {
	va_list	 ap;

	va_start(ap, str);
	vprintf(str, ap);
	puts("");
	va_end(ap);
}

void
log_info(const char *str, ...) {
	va_list	 ap;

	va_start(ap, str);
	vprintf(str, ap);
	puts("");
	va_end(ap);
}

void
log_debug(const char *str, ...) {
	va_list	 ap;

	va_start(ap, str);
	vprintf(str, ap);
	puts("");
	va_end(ap);
}

