/*
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef WINCOMPAT_H_
#define WINCOMPAT_H_
#if defined Windows

# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
# include <stdlib.h> /* For _countof */
# include <sys/types.h>

/* Avoid redifinition */
# if !defined __func__
#  define __func__ (char *)__FUNCTION__ /* Prevent a warning on Windows */
# endif

/*
 * Functions helpers
 */
# define alloca _alloca
# define strcpy(x, y) strcpy_s((x), _countof(x), (y))
/* From libtuntap's tuntap.h */
# undef snprintf
# undef _snprintf
# define snprintf(x, y, z, ...) _snprintf_s((x), (y), (y), (z), __VA_ARGS__);
# define strncat(x, y, z) strncat_s((x), _countof(x), (y), (z));

/*
 * Types helpers
 */
# define ssize_t SSIZE_T
typedef unsigned short sa_family_t;

/*
 * Prototypes our reimplementations
 */
long long strtonum(const char *, long long, long long, const char **);
char * strndup(const char *, size_t);
LPWSTR formated_error(LPWSTR pMessage, DWORD m, ...);

# endif
#endif
