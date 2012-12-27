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

#pragma once
#ifndef SUBSET_VO2UOWHN
#define SUBSET_VO2UOWHN

/*
 *
 * In this file, I defined the few macro I would like to have natively in C.
 *
 * At the moment:
 * 
 * - template <typename T> new<T>() -> T*;
 *
 */

#define tnt_new(TYPE) (TYPE *)tnt_new_impl(sizeof(TYPE))

void *tnt_new_impl(size_t sizeof_type);


#endif /* end of include guard: SUBSET_VO2UOWHN */
