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

#ifdef Windows
# define ZLIB_WINAPI
#endif
#include <zlib.h>

#include "compress.h"
#include "log.h"

uchar *
tnt_compress(uchar *in, const size_t in_size, size_t *out_size)
{
    int		error;
    uchar	*out;
    z_stream	strm;

    out = (unsigned char *)malloc((in_size < Z_MIN_SPACE ? Z_MIN_SPACE : in_size));
    if (out == NULL)
    {
        log_warn("do_compress");
        return NULL;
    }

    /* Initialisation */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    error = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (error != Z_OK)
    {
        log_warnx("zlib: error on deflateInit (%d)\n", error);
        return (NULL);
    }
    strm.avail_in = in_size;
    strm.next_in = (Bytef *)in;
    strm.avail_out = (in_size < Z_MIN_SPACE ? Z_MIN_SPACE : in_size);
    strm.next_out = (Bytef *)out;

    /* Compression */
    do
    {
        error = deflate(&strm, Z_FINISH);
        if (error != Z_OK)
            break;

	/* In this case, the compressed version is bigger */
	out = (unsigned char *)realloc(out, strm.total_out +
	  (in_size / 2 < Z_MIN_SPACE ? Z_MIN_SPACE : in_size / 2));
        if (out == NULL)
        {
            log_warn("do_compress");
            (void)deflateEnd(&strm);
            return NULL;
        }
        strm.next_out = (Bytef *)out + strm.total_out;
	if (in_size / 2 < Z_MIN_SPACE)
            strm.avail_out = in_size / 2;
        else
            strm.avail_out = Z_MIN_SPACE;
    } while (error == Z_OK);

    /* Should never happen */
    if (error != Z_STREAM_END)
    {
        log_warnx("zlib: deflating error\n");
        free(out);
        (void)deflateEnd(&strm);
        return NULL;
    }
    *out_size = strm.total_out;
    (void)deflateEnd(&strm);
    /* XXX: We should realloc here, adjusting the size */
    return out;
}

uchar *
tnt_uncompress(uchar *in, const size_t in_size, const size_t orignal_len)
{
    int          ret;
    uchar       *out;
    z_stream	 strm;

    out = (uchar *)malloc(orignal_len);
    if (out == NULL)
    {
        log_warn("do_uncompress");
        return NULL;
    }
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in;
    strm.avail_in = in_size;
    strm.next_out = out;
    strm.avail_out = orignal_len;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
    {
        log_warnx("zlib: error on inflateInit (%d)\n", ret);
        return NULL;
    }
    ret = inflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END)
    {
        log_warnx("zlib: error on inflate (%d)\n", ret);
        free(out);
        out = NULL;
    }
    (void)inflateEnd(&strm);
    return out;
}

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
    if ((compressed_data = realloc(compressed_data,
          *compressed_size + sizeof(size))) == NULL)
    {
        free(compressed_data);
        log_warn("reallocf failed. Sending uncompressed data");
        return NULL;
    }
    int n_size = htonl(size);
    memcpy(compressed_data + *compressed_size, &n_size,
      sizeof(n_size));
    *compressed_size += sizeof(n_size);
    return compressed_data;
}

uchar *tnt_uncompress_sized(uchar *compressed_data, const size_t size,
  size_t *uncompressed_size)
{
    int index = size - sizeof(int);
    printf("\n%d - %d = %d\n",size, sizeof(int), index);
    int net_size = *((int *)(compressed_data + (index)));
    int host_size = ntohl(net_size);
    /* i = memcpy(&i, compressed_data + (size - sizeof(int)), sizeof(int)); */
    /* i = ntohl(i); */
    printf("size is %d\n", host_size);
    *uncompressed_size = host_size;
    return tnt_uncompress(compressed_data, index, host_size);
}
