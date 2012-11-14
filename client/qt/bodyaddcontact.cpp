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
#include "bodyaddcontact.h"
#include <QWidget>

BodyAddContact::BodyAddContact(QWidget* parent, Controller* controller)
  : QFrame(parent), _controller(controller), _initialName("")
{
  this->setAttribute(Qt::WA_DeleteOnClose);

  setupUi(this);
  QObject::connect(okOrReject, SIGNAL(accepted()), _controller, SLOT(addContact()));
  QObject::connect(okOrReject, SIGNAL(rejected()), _controller, SLOT(unuseAddContact()));
  //QObject::connect(selectPubKey->button(QDialogButtonBox::Open), SIGNAL(clicked()), this, SLOT(openPubKey()));
}

BodyAddContact::BodyAddContact(QWidget* parent, Controller* controller, const QString& name, const QString& key)
  : QFrame(parent), _controller(controller), _initialName(name)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);
    QObject::connect(okOrReject, SIGNAL(accepted()), _controller, SLOT(addContact()));
    //QObject::connect(okOrReject, SIGNAL(rejected()), &win, SLOT(deleteAddContact()));
    this->name->setText(name);
    this->pubKey->setText(key);
}


BodyAddContact::~BodyAddContact()
{
}

QString BodyAddContact::getNewContactName() const
{
  return (this->name->text());
}

const QString BodyAddContact::getInitialContactName() const
{
  return this->_initialName;
}

QString BodyAddContact::getNewContactKey() const
{
  return (this->pubKey->toPlainText());
}

QString BodyAddContact::getContactIp() const
{
  return (this->ip->text());
}

void BodyAddContact::sendContact() {
    // Contact* newContact = new Contact(this->nameLabel->text().toStdString(),
    //                                   this->keyText->toPlainText().toStdString());
    // _clientGui->addContact(newContact);
}

void	BodyAddContact::openPubKey()
{
    if (_controller != 0)
        this->pubKey->setText(_controller->openPubKey());
}

void
BodyAddContact::cleanField()
{
    this->name->clear();
    this->pubKey->clear();
    this->ip->clear();
}
