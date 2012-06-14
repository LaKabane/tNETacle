#ifndef COMPRESS_UNIX_H_
# define COMPRESS_UNIX_H_



/*
 * This is almost the same e as in compress.h
 * Thoses functions append and read a size realtive to
 * the uncompressed lenght of the string
 */

uchar *tnt_compress_sized(uchar *, const int, size_t *);
uchar *tnt_uncompress_sized(uchar *, const size_t, size_t *);
#endif /* !COMPRESS_UNIX_H_ */
