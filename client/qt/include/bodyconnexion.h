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

#ifndef BODYCONNEXION_H
#define BODYCONNEXION_H

#include "ui_bodyconnexion.h"
#include "controller.h"
#include <QWidget>

namespace Ui {
    class BodyConnexion;
}

class BodyConnexion : public QFrame, public Ui::BodyConnexion
{
    Q_OBJECT
public:
    explicit BodyConnexion(QWidget* parent = 0, Controller* controller = 0);
    ~BodyConnexion();

    void    setName(const QString& name) { this->_name->setText(name);}
    void    setAdress(const QString& adress) { this->_adress->setText(adress);}
    void    setPort(const QString& port) { this->_port->setText(port);}
    void    setKey(const QString& key) { this->_pubKey->setText(key);}

    const QString    getName() const { return this->_name->text();}
    const QString    getAdress() const { return this->_adress->text();}
    const QString    getPort() const { return this->_port->text();}
    const QString    getKey() const { return this->_pubKey->toPlainText ();}

private:
    Controller*    _controller;
};

#endif // BODYCONNEXION_H
