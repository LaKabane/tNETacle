/**
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
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
**/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "server.h"
#include "frame.h"

void
frame_free(struct frame const *f)
{
    free(f->raw_packet);
}

int
frame_alloc(struct frame *frame,
            unsigned int size)
{
    void *tmp_raw_packet = NULL;
    void *tmp_frame_ptr = NULL;

    /* Alloc the size of the whole packet, plus the size of the header */
    tmp_raw_packet = (void *)malloc(size
                                    + sizeof(struct packet_hdr));
    if (tmp_raw_packet == NULL)
    {
        return -1;
    }
    /* Shift the frame to pointer to just behind the header */
    tmp_frame_ptr = (void *)((intptr_t)tmp_raw_packet
                             + sizeof(struct packet_hdr));

    /* Commit the results */
    frame->raw_packet = tmp_raw_packet;
    frame->frame = tmp_frame_ptr;
    frame->size = size;
    return 0;
}


