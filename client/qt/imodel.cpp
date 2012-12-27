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

#include <QMap>
#include <QVariant>
#include "imodel.h"
// const QMap<QString, QVariant> Model::mapToVar(const QMap<QString, QString> &map)
// {
//   QMap<QString, QVariant> var;
//   QMap<QString, QString>::const_iterator it;
//   const QMap<QString, QString>::const_iterator it_end = map.end();
//   for (; it != it_end; ++it)
//     {
//       QVariant str = it.value();
//       var[it.key()] = str;
//     }
//   return var;
// }


const QByteArray IModel::toJson() const
{
  // QJson::Serializer serializer;
  // QVariant name = QMap<QString, QVariant = this->getObjectName;
  // QVariant obj(this->getObjectName, Model::mapToVar(this->getData()));
  // return serializer.serialize(obj);

  QByteArray json;
  const QMap<QString, QVariant>* datas = this->getData();
  json += "{\n\"";
  json += this->getObjectName();
  json += "\":[";
  if (datas != 0)
    {
      QMap<QString, QVariant>::const_iterator it(datas->begin());
      for (const QMap<QString, QVariant>::const_iterator it_end = datas->end();
	   it != it_end; ++it)
	{
	  json += "{\nname:\"";
	  json += it.key();
	  json += "\"\nkey:\"";
	  json += it.value().toString();
	  json += "\"\n}\n,";
	}
    }
  json[json.size() - 1] = ' ';
  json += "]\n}";
  return json;
}
