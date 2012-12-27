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

#include <QDebug>
#include "exception.h"
#include "modelcontact.h"

const QString ModelContact::_name = "Contact";

ModelContact::ModelContact(Controller& controller):
  _contacts(),
  _controller(controller)
{
}

void  ModelContact::print()
{
}

void  ModelContact::feedData(const QString& command, const QVariant& data)
{
}

const QString& ModelContact::getObjectName() const
{
    return ModelContact::_name;
}

const QMap<QString, QVariant>* ModelContact::getData() const
{
    return &_contacts;
}

void ModelContact::addContact(peer* p)
{
    if (p == 0)
        throw new Exception("addContact: peer is not defined");
    QString name;
    QString key;
    QString ip;
    name.append(p->name);
    ip.append(p->ip);
    key.append(p->key);

    if (name.length() == 0)
        throw new Exception("Error: No name");
    if (_contacts.contains(name))
        throw new Exception("Error: Name already exist");
    QMap<QString, QVariant> tmp;
    tmp.insert("Key", QVariant(key));
    tmp.insert("Ip", QVariant(ip));
    _contacts[name] = QVariant(tmp);
}

const QString ModelContact::getKey(const QString &name)
{
    if (_contacts.contains(name) == 0)
        throw new Exception("Error: Name does not exist");
    return (_contacts[name].toMap())["Key"].toString();
}

const QString ModelContact::getIp(const QString &name)
{
    if (_contacts.contains(name) == 0)
        throw new Exception("Error: Name does not exist");
    return (_contacts[name].toMap())["Ip"].toString();
}

void ModelContact::delContact(const QVector<QString>& param)
{
    if (param.size() < 1)
        throw new Exception("Error: missing parameter to delete a contact");
    const QString& name = param[0];
    if (_contacts.remove(name) != 1)
      throw new Exception("Error: Name (" + name + ") does not exist");
}

void  ModelContact::editContact(const QVector<QString>& param)
{
    if (param.size() < 4)
        throw new Exception("Error: missing parameters to edit contact");
    const QString& name = param[0];
    const QString& key  = param[1];
    const QString& ip   = param[2];
    const QString& old  = param[3];

    QVector<QString> v;
    v.append(old);
    this->delContact(v);
    v.remove(0);
    v.append(name);
    v.append(key);
    v.append(ip);
    //this->addContact(v);
}

void ModelContact::clear()
{
  _contacts.clear();
}
