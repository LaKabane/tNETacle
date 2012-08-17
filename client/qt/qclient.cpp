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

#include "qclient.h"
#include "theader.h"
#include "bodyconnexion.h"
#include "bodyaddgroup.h"
#include "bodyaddcontact.h"
#include "bodymain.h"

#include <iostream>

QClient::QClient(Controller* controller) :
  QMainWindow(0),
  _controller(controller),
  _header(0),
  _body(0),
  _state(QClient::CONNEXION)
{
    setupUi(this);
    _header = new THeader(this, controller);
    _header->setObjectName(QString::fromUtf8("header"));
    _layout->addWidget(_header);

    _bodyConnexion = new BodyConnexion(this, controller);
    _bodyConnexion->setObjectName(QString::fromUtf8("bodyConnexion"));
    _bodyConnexion->show();

    _bodyMain = BodyMain::get(this, controller);
    _bodyMain->setObjectName(QString::fromUtf8("bodyMain"));
    _bodyMain->hide();

    _bodyAddGroup = new BodyAddGroup(this, controller);
    _bodyAddGroup->setObjectName(QString::fromUtf8("bodyAddGroup"));
    _bodyAddGroup->hide();

    _bodyAddContact = new BodyAddContact(this, controller);
    _bodyAddContact->setObjectName(QString::fromUtf8("bodyAddContact"));
    _bodyAddContact->hide();

    _body = _bodyConnexion;
    _layout->addWidget(_bodyConnexion);
    _layout->addWidget(_bodyMain);
    _layout->addWidget(_bodyAddGroup);
    _layout->addWidget(_bodyAddContact);
    _controller->setGui(this);
}

QClient::~QClient()
{
}

QWidget*
QClient::getHeader() const
{
    return _header;
}

QWidget*
QClient::getBody() const
{
    return _body;
}

void
QClient::changeNextBody(QClient::state newState)
{
    _body->hide();
    if (_state == QClient::CONNEXION)
    {
        _state = QClient::MAIN;
        _body = _bodyMain; 
    }
    else if (_state == QClient::MAIN)
    {
        if (newState == QClient::ADDCONTACT)
        {
            _state = QClient::ADDCONTACT;
            _body = _bodyAddContact;
        }
        else if (newState == QClient::ADDGROUP)
        {
            _state = QClient::ADDGROUP;
            _body = _bodyAddGroup;
        }
    }
    else if (_state == QClient::ADDGROUP)
    {
        _state = QClient::MAIN;
        _body = _bodyMain;
    }
    else if (_state == QClient::ADDCONTACT)
    {
        _state = QClient::MAIN;
        _body = _bodyMain;
    }
    _body->show();
}

void
QClient::changePrevBody()
{
    _body->hide();
    if (_state == QClient::MAIN)
    {
        _state = QClient::CONNEXION;
        _body = _bodyConnexion;

    }
    else if (_state == QClient::ADDGROUP)
    {
        _state = QClient::MAIN;
        _body = _bodyMain;
    }
    else if (_state == QClient::ADDCONTACT)
    {
        _state = QClient::MAIN;
        _body = _bodyMain;
    }
    _body->show();
}

void
QClient::connected()
{
    changeNextBody();
}

void
QClient::disconnected()
{
    _state = QClient::CONNEXION;
    _body->hide();
    _body = _bodyConnexion;
    _body->show();
}
