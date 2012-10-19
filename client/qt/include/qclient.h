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

#ifndef QCLIENT_H_
# define QCLIENT_H_

#include <QMainWindow>
#include "controller.h"
#include "iclientgui.h"
#include "ui_qclient.h"

namespace Ui {
    class QClient;
}

class QClient : public QMainWindow, public Ui::QClient, public IClientGUI
{
    Q_OBJECT

    enum state
    {
        CONNEXION,
        MAIN,
        ADDGROUP
    };

public:
    explicit   QClient(Controller* controller = 0);
    virtual    ~QClient();
    QWidget*    getHeader() const;
    QWidget*    getBody() const;
    void        changeNextBody();
    void        changePrevBody();
    void        connected();
    void        disconnected();

private:
    Controller*    _controller;
    QWidget*    _header;
    QWidget*    _body;
    state       _state;

    QWidget*    _bodyConnexion;
    QWidget*    _bodyMain;
    QWidget*    _bodyAddGroup;
};

#endif /* !QCLIENT_H_ */
