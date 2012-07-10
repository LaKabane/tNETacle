/**
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**/


OVERLAPPED gl_overlap;

ssize_t
windows_fix_write(intptr_t fd, void *buf, size_t len)
{
	DWORD n = 0;
	int err;
	err = WriteFile((HANDLE)fd, buf, len, &n, &gl_overlap);
	if (err == 0 && GetLastError() != ERROR_IO_PENDING)
	{
		printf("Write failed, error %d\n", GetLastError());
		return -1;
	}
	else
	{
		if (len > 0)
		{
			log_debug("== WRITE == WRITE == WRITE == WRITE ==");
			hex_dump_chk(buf, len);
			log_debug("== WRITE == WRITE == WRITE == WRITE ==");
		}
		WaitForSingleObject(gl_overlap.hEvent, INFINITE);
		return n;
	}
}

ssize_t
windows_fix_read(intptr_t fd, void *buf, size_t len)
{
	DWORD n = 0;
	int err;

	err = ReadFile((HANDLE)fd, buf, len, &n, &gl_overlap);
	if (err == 0 && GetLastError() != ERROR_IO_PENDING)
	{
		printf("Read failed, error %d\n", GetLastError());
		return -1;
	}
	else
	{
		if (n > 0)
		{
			log_debug("== READ == READ == READ == READ ==");
			hex_dump_chk(buf, n);
			log_debug("== READ == READ == READ == READ ==");
		}
		WaitForSingleObject(gl_overlap.hEvent, 1000);
		return n;
	}
}

