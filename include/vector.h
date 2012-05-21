/**
 * Copyright (c) 2012, PICHOT Fabien Paul Leonard <pichot.fabien@gmail.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
**/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#ifndef DEFAULT_ALLOC_SIZE
#define default_alloc_size 64
#else
#define default_alloc_size DEFAULT_ALLOC_SIZE
#endif

#ifndef VECTOR_TYPE
#error "You must define the vector type via the VECTOR_TYPE macro prior to include vector.h"
#else
#define type VECTOR_TYPE
#endif

#ifndef VECTOR_PREFIX
#error "You must define the macro VECTOR_PREFIX prior to include vector.h"
#else
#define prefix VECTOR_PREFIX
#endif

#ifndef TYPE_SPECIFIER
#if __STDC_VERSION__ >= 199901L
#define specifier static inline
#else
#define specifier static
#endif
#else
#define specifier TYPE_SPECIFIER
#endif

#define _XTYPE_NAME(name, pr) name ## pr
#define _TYPE_NAME(name, pr) _XTYPE_NAME(name, pr)
#define vector_name _TYPE_NAME(vector_, VECTOR_PREFIX)

struct vector_name {
  VECTOR_TYPE *vec;
  size_t size;
  size_t alloc_size;
};

#define NAME__(p, t, name) p ## _ ## t ## _ ## name
#define NAME_(p, t, name) NAME__(p, t, name)
#define vector_(name) NAME_(v, VECTOR_PREFIX, name)

specifier void vector_(init)(struct vector_name *v);
specifier void vector_(push)(struct vector_name *v, type *val);
specifier int vector_(resize)(struct vector_name *v, size_t);
specifier void vector_(insert_range)(struct vector_name *, type *,
                                           type *, type *);
specifier void vector_(insert)(struct vector_name *, type *, type *);
specifier void vector_(pop)(struct vector_name *v);
specifier void vector_(delete)(struct vector_name *v);
specifier type *vector_(begin)(struct vector_name *v);
specifier type *vector_(end)(struct vector_name *v);
specifier type *vector_(next)(type *it);
specifier void vector_(erase)(struct vector_name *v, type *ptr);
specifier void vector_(erase_range)(struct vector_name *v, type *,
                                          type*);
specifier type vector_(at)(struct vector_name *v, size_t i);
specifier type *vector_(atref)(struct vector_name *v, size_t i);
specifier type vector_(front)(struct vector_name *v);
specifier type vector_(back)(struct vector_name *v);
specifier type *vector_(frontref)(struct vector_name *v);
specifier type *vector_(backref)(struct vector_name *v);
specifier type *vector_(find_if)(struct vector_name *v, type *val,
                                      int (*)(type const *, type const *));

#ifdef VECTOR_TYPE_SCALAR
specifier type *vector_(find)(struct vector_name *v, type *val);
#endif


specifier void vector_(init)(struct vector_name  *v)
{
  v->size = 0;
  v->alloc_size = default_alloc_size;
  v->vec = (type*)calloc(v->alloc_size, sizeof(type));
}

specifier void vector_(push)(struct vector_name  *v, type *val)
{
  if (v->size < v->alloc_size) {
    v->vec[v->size] = *val;
    v->size += 1;
  } else {
    int err;
    err = vector_(resize)(v, default_alloc_size);
    if (err != -1)
    {
      v->vec[v->size] = *val;
      v->size += 1;
    }
  }
}

specifier int vector_(resize)(struct vector_name *v, size_t size)
{
    size_t next_alloc_size;
    void *tmpptr;
    /*check for overflow*/
    if (((v->alloc_size + size) * sizeof(type)) < v->alloc_size){
      next_alloc_size = SIZE_MAX;
    }
    else{
      next_alloc_size = (v->alloc_size + size) * sizeof(type);
    }
    tmpptr = realloc(v->vec, next_alloc_size);
    if (tmpptr != NULL)
    {
      v->vec = tmpptr;
      v->alloc_size = next_alloc_size / sizeof(type);
      return 0;
    }
    return -1;
}

specifier void vector_(insert)(struct vector_name *v, type *at, type *val)
{
  if ((at) <= (v->vec + v->size)
    && (at >= v->vec))
    {
      if (v->alloc_size - v->size == 0)
      {
        /*Fuck ! We need to resize for only one element...*/
        int err;
        size_t offset = at - v->vec;

        err = vector_(resize)(v, default_alloc_size);
        if (err == -1)
          return ;
        at = &v->vec[offset];
      }
      size_t number_to_move = vector_(end)(v) - at;
      memmove(at + 1, at, number_to_move * sizeof(type));
      *at = *val;
      v->size++;
    }
}

specifier void vector_(insert_range)(struct vector_name *v, type *at,
                                           type *from, type *to)
{
  if ((at <= (v->vec + v->size))
      && (at >= v->vec))
  {
    size_t inserted_size = (to - from);
    if ((v->size + inserted_size) > v->alloc_size)
    {
      /*we need to resize*/
      int err;
      size_t offset = at - v->vec;
      err = vector_(resize)(v, inserted_size + default_alloc_size);
      if (err == -1)
        return ;
      at = &v->vec[offset];
    }
    size_t number_to_move = vector_(end)(v) - at;
    if (number_to_move != 0)
    {
      memmove(at + inserted_size, at, number_to_move * sizeof(type));
    }
    memmove(at, from, inserted_size * sizeof(type));
    v->size += inserted_size;
  }
}

specifier void vector_(pop)(struct vector_name *v)
{
  type *it = &v->vec[v->size - 1];
  vector_(erase)(v, it);
}

specifier void vector_(delete)(struct vector_name  *v)
{
  if (v->vec != NULL) {
    free(v->vec);
  }
}

specifier type * vector_(begin)(struct vector_name  *v)
{
  return v->vec;
}

specifier type * vector_(end)(struct vector_name  *v)
{
  return &v->vec[v->size];
}

specifier type * vector_(next)(type *it)
{
  return ++it;
}
specifier type vector_(at)(struct vector_name *v, size_t i)
{
  return v->vec[i];
}
specifier type *vector_(atref)(struct vector_name *v, size_t i)
{
  return &v->vec[i];
}
specifier type vector_(front)(struct vector_name *v)
{
  return v->vec[0];
}
specifier type vector_(back)(struct vector_name *v)
{
  if (v->size != 0)
    return v->vec[v->size - 1];
  else
    return v->vec[0];
}
specifier type *vector_(frontref)(struct vector_name *v)
{
  return &v->vec[0];
}
specifier type *vector_(backref)(struct vector_name *v)
{
  if (v->size != 0)
    return &v->vec[v->size - 1];
  else
    return &v->vec[0];
}
specifier void vector_(erase)(struct vector_name *v, type *ptr)
{
  if (ptr > (v->vec + v->size))
  {
    return ;
  } else {
    size_t n = 0;
    type *dest = ptr;
    type *src = ptr + 1;
    n = (uintptr_t)&v->vec[v->size] - (uintptr_t)src;
    memmove(dest, src, n);
    v->size--;
  }
}
specifier void vector_(erase_range)(struct vector_name *v,
                                          type *from, type* to)
{
  if (from > (v->vec + v->size))
    return ;
  if (to == vector_(end)(v))
  {
    /*don't erase anything, just set the size*/
    size_t new_size = from - v->vec;
    v->size = new_size;
  } else {
    size_t move_size = vector_(end)(v) - to;
    memmove(from, to, move_size);
  }
}

specifier type *vector_(find_if)(struct vector_name *v, type *ptr,
                                       int (*cmp)(type const *, type const *))
{
  type* it = NULL;
  type* ite = NULL;

  for (it = vector_(begin)(v),
       ite = vector_(end)(v);
       it != ite;
       it = vector_(next)(it)) {
    if (cmp(it, ptr)) {
      break;
    }
  }
  return it;
}

#ifdef VECTOR_TYPE_SCALAR

specifier type * vector_(find)(struct vector_name *v, type *ptr)
{
  type* it = NULL;
  type* ite = NULL;

  for (it = vector_(begin)(v),
       ite = vector_(end)(v);
       it != ite;
       it = vector_(next)(it)) {
    if (it == ptr) {
      break;
    }
  }
  return it;
}

#endif

#undef type
#undef specifier
#undef prefix
