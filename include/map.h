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

#ifndef MAP_KEY_TYPE
#error "You must define the key type via the MAP_KEY_TYPE macro before including map.h"
#else
#define key_type MAP_KEY_TYPE
#endif

#ifndef MAP_VALUE_TYPE
#error "You must define the value type via the MAP_VALUE_TYPE macro before including map.h"
#else
#define value_type MAP_VALUE_TYPE
#endif

#ifndef MAP_PREFIX
#error "You must define the macro MAP_PREFIX prior to include map.h"
#endif

#ifndef TYPE_SPECIFIER
#if __STDC_VERSION__ >= 199901L
#define map_specifier static inline
#else
#define map_specifier static
#endif
#else
#define map_specifier TYPE_SPECIFIER
#endif

#define _XTYPE_NAME(name, pr) name ## pr
#define _TYPE_NAME(name, pr) _XTYPE_NAME(name, pr)
#define map_name _TYPE_NAME(map_, MAP_PREFIX)
#define map_pair_name _TYPE_NAME(map_pair_, MAP_PREFIX)
#define cmp_name _TYPE_NAME(map_pair_cmp_, MAP_PREFIX)

#define NAME__(p, t, name) p ## _ ## t ## _ ## name
#define NAME_(p, t, name) NAME__(p, t, name)
#define map_(name) NAME_(m, MAP_PREFIX, name)

struct map_pair_name {
  key_type    key;
  value_type  value;
};

#define VECTOR_TYPE struct map_pair_name
#define VECTOR_PREFIX MAP_PREFIX
#define VECTOR_DEV_MODE
#include "vector.h"

struct map_name {
  struct vector_name *vec;
};

map_specifier struct map_name *map_(new)(void);
map_specifier value_type *map_(insert)(struct map_name *m, key_type k, value_type v);
map_specifier value_type const *map_(access)(struct map_name *m, key_type k);
map_specifier value_type *map_(find)(struct map_name *m, key_type k);
map_specifier void map_(delete)(struct map_name *m);
map_specifier void map_(remove)(struct map_name *m, key_type k);

map_specifier struct map_name *map_(new)(void)
{
    struct map_name *tmp;

    tmp = (struct map_name *)malloc(sizeof(struct map_name));
    if (tmp == NULL)
        return NULL;
    tmp->vec = vector_(new)();
    return tmp;
}

#ifdef MAP_KEY_CMP
map_specifier int cmp_name(struct map_pair_name const *a, struct map_pair_name const *b)
{
  return MAP_KEY_CMP(&a->key, &b->key);
}

#else

map_specifier int cmp_name(struct map_pair_name const *a, void *c)
{
    struct map_pair_name *b = (struct map_pair_name *)c;

    return a->key == b->key;
}
#endif
map_specifier value_type *map_(insert)(struct map_name *m, key_type k, value_type v)
{
  struct map_pair_name *it;
  struct map_pair_name pair = {
    .key = k,
    .value = v,
  };

  it = vector_(find_if)(m->vec, cmp_name, &pair);
  if (it == vector_(end)(m->vec))
    vector_(push)(m->vec, &pair); // A copy is made inside, so don't worry !
  else
  {
    it->value = v;
  }
  return &it->value;
}

map_specifier void map_(remove)(struct map_name *m, key_type k)
{
  struct map_pair_name *it;
  struct map_pair_name pair = {
    .key = k,
  };

  it = vector_(find_if)(m->vec, cmp_name, &pair);
  if (it == vector_(end)(m->vec))
    return;
  else
  {
    vector_(erase)(m->vec, it);
  }
}

map_specifier value_type const *map_(access)(struct map_name *m, key_type k)
{
  struct map_pair_name search = {
    .key = k,
  };
  struct map_pair_name *it;

  it = vector_(find_if)(m->vec, cmp_name, &search);
  if (it == vector_(end)(m->vec))
    return NULL;
  else
    return &it->value;
}

map_specifier value_type *map_(find)(struct map_name *m, key_type k)
{
  struct map_pair_name search = {
    .key = k,
  };
  struct map_pair_name *it;

  it = vector_(find_if)(m->vec, cmp_name, &search);
  if (it == vector_(end)(m->vec))
    return NULL;
  else
    return &it->value;
}

map_specifier int map_(count)(struct map_name *m)
{
  return m->vec->size;
}

map_specifier void map_(delete)(struct map_name *m)
{
  vector_(delete)(m->vec);
}

#undef vector_
#undef vector_name
#undef DEFAULT_ALLOC_SIZE
#undef VECTOR_TYPE
#undef VECTOR_PREFIX
#undef VECTOR_TYPE_SCALAR
#undef MAP_KEY_TYPE
#undef MAP_KEY_CMP
#undef MAP_VALUE_TYPE
#undef MAP_PREFIX
#undef map_specifier
#undef map_
#undef map_name
#undef map_pair_name
#undef value_type
#undef key_type
