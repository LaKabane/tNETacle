/*
 * Copyright (c) 2012 Florent Tribouilloy <tribou_f AT epitech DOT net>
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

#ifndef UTILS_H_
# define UTILS_H_

#include <QVariant>
//#include "tclt_json.h"

class Utils
{
public:
    static QVariant* getVariant(const char *buf, size_t len);

/*	static QVariant createVariantSimple(elements **e, bool &ok);
	static QVariant createVariantArray(elements **e, bool &ok);
	static QVariant createVariantMap(elements **e, bool &ok);
	static QVariant createVariantWithKey(elements **e, bool &ok);
    static QVariant* createVariant(elements *e);*/
};
/*
struct s_elementType
{
	enum element_type type;
	QVariant (*fun)(elements **e, bool &ok);
};

typedef struct s_elementType eType;
*/
#endif /* !UTILS_H_ */
