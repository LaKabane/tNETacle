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

#ifndef MODELCONTACT_H_
# define MODELCONTACT_H_

#include <QMap>
#include <QString>
#include "imodel.h"

class Controller;

class ModelContact : public IModel
{
  typedef void (ModelContact::*fun)(const QVector<QString>&);
  typedef QMap<QString, fun> mapfun;

public:
  ModelContact(Controller &);
  virtual       ~ModelContact(){}

  void          addContact(const QVector<QString>&);
  const QString getKey(const QString &);
  const QString getIp(const QString &);
  void          delContact(const QVector<QString>&);
  void          editContact(const QVector<QString>&);
  void		print();
  void		clear();

  virtual void  feedData(const QString &,const QVariant&);
  virtual const QString &getObjectName() const;

private:
  QMap<QString, QVariant> _contacts;
  Controller    &_controller;
  static const QString _name;
  static mapfun	_commands;

protected: // from IModel
  virtual const QMap<QString, QVariant>* getData() const;

};

#endif /* !MODELCONTACT_H */
