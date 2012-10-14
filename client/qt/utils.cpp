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

#include "utils.h"
#include "exception.h"

eType types[] =
	{
		{E_START_MAP, &Utils::createVariantMap},
		{E_START_ARRAY, &Utils::createVariantArray},
	};

QVariant
Utils::createVariantSimple(elements** e, bool&)
{
	QVariant qvar;

	if (e != 0 && *e != 0)
	{
		qvar.setValue(QString((*e)->u_value.buf));
	}
	return qvar;
}

QVariant
Utils::createVariantMap(elements** e, bool& ok)
{
    QMap<QString, QVariant> qmap;

    if (e != 0)
    {
        while (*e != NULL && (*e)->type != E_NOTHING && (*e)->type != E_END_MAP)
        {
            if ((*e)->type == 8)
                exit(1);
	    if (*e != 0 && (*e)->type == E_MAP_KEY)
            {
                QString key((*e)->u_value.buf);
                *e = (*e)->next;
                if ((*e)->type == E_START_ARRAY || (*e)->type == E_START_MAP)
                {
                    for (unsigned int i=0; i < (sizeof(types) / sizeof(eType)); ++i)
                    {
                        if (types[i].type == (*e)->type)
                        {
                            *e = (*e)->next;
                            QVariant ret = types[i].fun(e, ok);
                            qmap[key] = ret;
                            break;
                        }
                    }
                }
                else if ((*e)->type != E_NOTHING)
                {
                    QVariant ret = Utils::createVariantSimple(e, ok);
                    qmap[key] = ret;
                    *e = (*e)->next;
                }
                else if (*e != NULL)
                    *e = (*e)->next;
            }
        }
        if (*e != NULL && (*e)->type == E_END_MAP)
        {
            *e = (*e)->next;
            return qmap;
        }
    }
    ok = false;
    return qmap;
}

QVariant
Utils::createVariantArray(elements** e, bool& ok)
{
	QVariantList qlist;

	if (e != 0)
	{
		while (*e != NULL && (*e)->type != E_NOTHING && (*e)->type != E_END_ARRAY)
		{
			if ((*e)->type == E_START_ARRAY || (*e)->type == E_START_MAP)
			{
				for (unsigned int i=0; i < (sizeof(types) / sizeof(eType)); ++i)
				{
					if (types[i].type == (*e)->type)
					{
						*e = (*e)->next;
						QVariant ret = types[i].fun(e, ok);
						qlist.append(ret);
						break;
					}
				}

			}
			else
			{
				QVariant ret = Utils::createVariantSimple(e, ok);
				qlist.append(ret);
				*e = (*e)->next;
			}
		}
		if ((*e) != NULL && (*e)->type == E_END_ARRAY)
		{
			*e = (*e)->next;
			return qlist;
		}
	}
	ok = false;
	return qlist;
}

QVariant*
Utils::createVariant(elements* e)
{
	QVariant *qvar = 0;
	elements *tmp = e;

	if (tmp != NULL && tmp->type != E_NOTHING)
	{
		if (tmp->type == E_START_ARRAY || tmp->type == E_START_MAP)
		{
			for (unsigned int i=0; i < sizeof(types) / sizeof(eType); ++i)
			{
				if (types[i].type == tmp->type)
				{
					tmp = tmp->next;
					bool ok = true;
					QVariant ret = types[i].fun(&tmp, ok);
					if (ok == false)
					{
						throw new Exception("Problem to parse the elements list");
					}
					qvar = new QVariant(ret);
					if (qvar == 0)
					{
						throw new Exception("Error to allocate a new QVariant");
					}
					break;
				}
			}
		}
	}
	if (tmp != NULL && tmp->type != E_NOTHING)
	{
		throw new QString("Error, buffer received not at the end");
	}
	return qvar;
}

QVariant*
Utils::getVariant(const char* buf, size_t len)
{
	QVariant* qvar = 0;
	elements* e = tclt_parse(buf, len);

	if (e == 0)
		return 0;

	qvar = Utils::createVariant(e);
	return qvar;
}
