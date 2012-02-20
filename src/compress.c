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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
# define ZLIB_WINAPI
#endif
#include <zlib.h>

#include "compress.h"
#include "log.h"

t_string *
tnt_compress(t_string *in)
{
    int		error;
    t_string	*out;
    z_stream	strm;

    out = (t_string *)malloc(sizeof(*out));
    out->data = (unsigned char *)malloc((in->size < Z_MIN_SPACE ? Z_MIN_SPACE : in->size));
    if (out == NULL || out->data == NULL)
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
    strm.avail_in = in->size;
    strm.next_in = (Bytef *)in->data;
    strm.avail_out = (in->size < Z_MIN_SPACE ? Z_MIN_SPACE : in->size);
    strm.next_out = (Bytef *)out->data;

    /* Compression */
    do
    {
        error = deflate(&strm, Z_FINISH);
        if (error != Z_OK)
            break;

	/* In this case, the compressed version is bigger */
	out->data = (unsigned char *)realloc(out->data, strm.total_out +
	  (in->size / 2 < Z_MIN_SPACE ? Z_MIN_SPACE : in->size / 2));
        if (out->data == NULL)
        {
            log_warn("do_compress");
            (void)deflateEnd(&strm);
            return NULL;
        }
        strm.next_out = (Bytef *)out->data + strm.total_out;
	if (in->size / 2 < Z_MIN_SPACE)
            strm.avail_out = in->size / 2;
        else
            strm.avail_out = Z_MIN_SPACE;
    } while (error == Z_OK);

    /* Should never happen */
    if (error != Z_STREAM_END)
    {
        log_warnx("zlib: deflating error\n");
        free(out->data);
        free(out);
        (void)deflateEnd(&strm);
        return NULL;
    }
    out->size = strm.total_out;
    (void)deflateEnd(&strm);
    /* XXX: We should realloc here, adjusting the size */
    return out;
}

t_string *
tnt_uncompress(t_string *in, const size_t orignal_len)
{
	int error;
    int		ret;
    t_string	*out;
    z_stream	strm;

    out = (t_string *)malloc(sizeof(*out));
    out->data = (unsigned char *)malloc(orignal_len);
    if (out == NULL || out->data == NULL)
    {
        log_warn("do_uncompress");
        return NULL;
    }
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in->data;
    strm.avail_in = in->size;
    strm.next_out = out->data;
    strm.avail_out = orignal_len;
    out->size = orignal_len;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
    {
        log_warnx("zlib: error on inflateInit (%d)\n", error);
        return NULL;
    }
    ret = inflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END)
    {
        log_warnx("zlib: error on inflate (%d)\n", error);
        free(out->data);
        free(out);
        out = NULL;
    }
    (void)inflateEnd(&strm);
    return out;
}
//
//int	main(int ac, const char **av)
//{
//  HANDLE	fd;
//  HANDLE map;
//  int tmp;
//  t_string original;
//  t_string *compressed;
//  t_string *uncompressed;
//
//  if (!av[1])
//    {
//      log_warn("Usage: ./a.out filename\n");
//      return (EXIT_FAILURE);
//    }
//	fd = CreateFile(av[1], GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
//  if (fd == INVALID_HANDLE_VALUE)
//    {
//      log_warn("Pas de fichier\n");
//      return (-1);
//    }
//  original.size = GetFileSize(fd, (LPDWORD)(&tmp));
////  lseek(fd, 0, SEEK_SET);
//  map = CreateFileMapping(fd, NULL, PAGE_READWRITE, 0, 0, "filemap");
//  // mmap == mapViewOfFile()
//  original.data = (unsigned char *)MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, original.size);//mmap(NULL, original.size, PROT_READ, MAP_PRIVATE, fd, 0);
//  CloseHandle(fd);
//
//  compressed = tnt_compress(&original); // haaaan il verifie pas la valeur de retour!
//  if (compressed == NULL)
//	  printf("mais bordel !\n");
//  printf("Orginal size was %lu ... new one is %lu\n", original.size, compressed->size);
//  uncompressed = tnt_uncompress(compressed, original.size);
//  printf("Orginal size was %lu ... restored size is: %lu difference is %lu\n",
//	 original.size, uncompressed->size, original.size - uncompressed->size);
//    if (original.size == uncompressed->size)
//    printf(memcmp(original.data, uncompressed->data, original.size) ?
//		  "Diff KO\n": "Diff OK\n");
//
//	UnmapViewOfFile(original.data);
//	CloseHandle(map);
//	//	munmap(original.data, original.size);
//  printf("\nRatio is %lf%%\n", ((float)(original.size - compressed->size)) / original.size * 100);
//  free(compressed->data);
//  free(compressed);
//  free(uncompressed->data);
//  free(uncompressed);
//  return (EXIT_SUCCESS);
//}
