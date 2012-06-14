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

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "compress.h"
#include "compress_unix.h"
#include "log.h"



uchar *tnt_compress_sized(uchar *uncompressed_data, const int size,
  size_t *compressed_size)
{
    uchar *compressed_data = tnt_compress(uncompressed_data, size, compressed_size);
    if (compressed_data == NULL)
    {
        log_warn("Compress failed. Sending uncompressed data.");
        return NULL;
    }
    log_debug("Compressed from %d to %d + int size.",
      size, *compressed_size);
    if ((compressed_data = reallocf(compressed_data,
          *compressed_size + sizeof(size))) == NULL)
    {
        log_warn("reallocf failed. Sending uncompressed data");
        return NULL;
    }
    memcpy(compressed_data + *compressed_size, &size,
      sizeof(size));
    compressed_size += sizeof(size);
    return compressed_data;
}

uchar *tnt_uncompress_sized(uchar *compressed_data, const size_t size,
  size_t *uncompressed_size)
{
    *uncompressed_size = ntohl((int)
      (compressed_data + size - sizeof(int)));
        return tnt_uncompress(compressed_data,
      size - sizeof(uncompressed_size), *uncompressed_size);
}
