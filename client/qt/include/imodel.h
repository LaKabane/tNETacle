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

#ifndef MODEL_H_
# define MODEL_H_


// This class is empty but shoudl allow us to add common features to mode objhects
// after disscution, toward communication with core features.
// (exemple: export in json features)

#include <QString>
#include <QMap>
#include <QByteArray>
#include <QVariant>

class IModel
{
public:
  const QByteArray toJson() const;//For debug only
  virtual const QString &getObjectName() const  = 0;
  virtual void  feedData(const QString &, const QVariant&) = 0;
  virtual const QMap<QString, QVariant>* getData() const = 0;
private:
  //  static const QMap<QString, QVariant> mapToVar(const QMap<QString, QString> &);
};

#endif /* !MODEL_H_ */
