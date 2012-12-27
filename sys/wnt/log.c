/* TODO: Windows NT logging API implementation */

#include <stdio.h>
#include "log.h"
#include "wincompat.h"

void
log_set_prefix(char *s) {
	(void)s;
}

void
log_set_filter(int max_pri) {
	(void)max_pri;
}

void
log_init(void) {
}

void
log_err(int eval, const char *str, ...) {
    va_list	 ap;

    va_start(ap, str);
    vprintf(str, ap);
    puts((char *)formated_error(L"%1%0", GetLastError()));
    va_end(ap);
    /* Abort for now */
    (void)eval;
    abort();
}

void
log_errx(int eval, const char *str, ...) {
	va_list	 ap;

	va_start(ap, str);
	vprintf(str, ap);
  	puts("\n");
    va_end(ap);
    /* Abort for now */
    (void)eval;
    abort();
}

void
log_warn(const char *str, ...) {
	va_list	 ap;

	va_start(ap, str);
	vprintf(str, ap);
    puts((char *)formated_error(L"%1%0", GetLastError()));
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

