/*
 * Copyright (c) 2011 Antoine Marandon <ntnmrndn AT gmail DOT com>
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

#ifndef COMPRESS_H
#define COMPRESS_H

#define Z_MIN_SPACE 6


typedef unsigned char uchar;

/* Return a compressed string or NULL in case of error. */
/* First parameter is data to compress */
/* Second parameter is size of the data to compress */
/* Third parameter is size of the returned string. */

uchar *tnt_compress(uchar *, const size_t, size_t *);

/* Return an uncompressed string or NULL in case of error. */
/* First parameter is data to uncompress */
/* Second parameter is size of the data to uncompress */
/* Third parameter is size of the orginal string */


uchar *tnt_uncompress(uchar *, const size_t, const size_t);

#endif /* !COMPRESS_H */
