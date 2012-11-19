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

#include "bodymain.h"

QWidget* BodyMain::_instance = 0;

BodyMain::BodyMain(QWidget *parent, Controller* controller) :
    QFrame(parent), _controller(controller)
{
    this->setupUi(this);
    _listContacts->clear();
    _listGroups->clear();

    QObject::connect(this->_addPeer, SIGNAL(clicked()), _controller, SLOT(viewAddContact()));
    QObject::connect(this->_deletePeer, SIGNAL(clicked()), this, SLOT(deleteContact()));
}

void
BodyMain::addNewPeer(const QString& str)
{
    QTreeWidgetItem* newGuiPeer = new QTreeWidgetItem(_listContacts);
    newGuiPeer->setText(0, str);
}

void
BodyMain::deleteContact()
{
    QList<QTreeWidgetItem*> peers = _listContacts->selectedItems();
    for (unsigned int i = 0; i < peers.size(); ++i)
    {
        QTreeWidgetItem* peer = peers[i];
        QString name = peer->text(0);
        //_controller->deleteContact(name);
        qDebug() << "lol";
        _listContacts->removeItemWidget(peer, 0);
    }
}

QWidget*
BodyMain::get(QWidget* parent, Controller* c)
{
    if (BodyMain::_instance == 0)
        BodyMain::_instance = new BodyMain(parent, c);
    return BodyMain::_instance;
}
