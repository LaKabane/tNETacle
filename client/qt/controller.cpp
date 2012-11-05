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
#include <QRegExp>
#include "iclientgui.h"
#include "controller.h"
#include "protocol.h"
#include "exception.h"
#include "imodel.h"
#include "bodyconnexion.h"
#include "tclt.h"
#include "tclt_command.h"
#include <iostream>

Controller::Controller() :
  _view(0),
  _network(*this),
  _models()
{
    _modelContacts = new ModelContact(*this);
    _modelNode = new ModelRootNode(*this);
    _modelLog = new ModelLog(*this);
    _modelConfig = new ModelConfig(*this);
    _modelConnexion = new ModelConnexion(this);

    _models[_modelContacts->getObjectName()] = _modelContacts;
    _models[_modelNode->getObjectName()] = _modelNode;
    _models[_modelLog->getObjectName()] = _modelLog;
    _models[_modelConfig->getObjectName()] = _modelConfig;
    _models[_modelConnexion->getObjectName()] = _modelConnexion;

    this->init_callback();
}

void Controller::feedData(const QVariant& data)
{
}

int
Controller::add_peer_controll(void *f)
{
    peer *p = static_cast<peer*>(f);

    std::cout << "add peer :" << p->name << " " << p->ip << " " << p->key << std::endl;
    return 0;
}

int
Controller::add_log_controll(void *f)
{
    char *str = static_cast<char*>(f);

    std::cout << "add log :" << str << std::endl;
    return 0;
}

int
Controller::delete_peer_controll(void *f)
{
    char *str = static_cast<char*>(f);

    std::cout << "delete peer :" << str << std::endl;
    return 0;
}

int
Controller::edit_peer_controll(void *f)
{
    std::cout << "edit peer" << std::endl;
    return 0;
}

void
Controller::init_callback()
{
    tclt_set_callback_command(ADD_PEER_CMD, add_peer_controll);
    tclt_set_callback_command(ADD_LOG_CMD, add_log_controll);
    tclt_set_callback_command(DELETE_PEER_CMD, delete_peer_controll);
    tclt_set_callback_command(EDIT_PEER_CMD, edit_peer_controll);
}

void Controller::appendLog(const QString& s)
{
    //this->_view->appendLog(s);
}

void Controller::editContact(QListWidgetItem* item)
{
    try {
        /*this->_view->createAddContact(item->text(),
                                      dynamic_cast<ModelContact*>(_modelContacts)->getKey(item->text()),
                                      dynamic_cast<ModelContact*>(_modelContacts)->getIp(item->text()));*/
    }
    catch (Exception *e)
    {
        //this->_view->printError(e->getMessage());
        delete e;
    }
}

const QString Controller::getIp() const
{
    return dynamic_cast<ModelRootNode*>(_modelNode)->getIP();
}

quint16 Controller::getPort() const
{
    return dynamic_cast<ModelRootNode*>(_modelNode)->getPort().toUShort();
}

void Controller::error(const QString &s)
{
    //this->_view->printError(s);
}

void Controller::deleteContact()
{
    try
    {
        QVector<QString> v;
        //v.append(_view->getSelected());
        dynamic_cast<ModelContact*>(this->_modelContacts)->delContact(v);
        this->writeToSocket(Protocol::delet(dynamic_cast<ModelContact*>(this->_modelContacts)->getObjectName(), v));
    }
    catch (Exception *e)
    {
        error(e->getMessage());
	delete e;
    }
}

bool Controller::addContact()
{
    /*QString pubkey = this->_view->getNewContactKey();
    QString name = this->_view->getNewContactName();
    QString ip = this->_view->getContactIp();*/

    /*if (name == "" || pubkey == "" || ip == "")
    {
        error("One of the mandatory fields is missing");
        return false;
    }
    if (checkName(name) == false)
    {
        error("the name is not correctly formated (only alpha-numeric and '_' characters are allowed)");
        return false;
    }
    // if (checkIP(ip) == false)
    // {
    //     error("the IP is not correctly formated");
    //     return false;
    // }

    if (!_view->getInitialContactName().isEmpty()) // TODO if user select a different row after editing
        this->deleteContact();
    try
    {
        QVector<QString> v;
        v.append(name);
        v.append(pubkey);
        v.append(ip);
        dynamic_cast<ModelContact*>(this->_modelContacts)->addContact(v);
        this->writeToSocket(Protocol::add(dynamic_cast<ModelContact*>(this->_modelContacts)->getObjectName(), v));
    }
    catch (Exception *e)
    {
        error(e->getMessage());
        delete e;
        return false;
    }
    //this->_view->deleteAddContact();*/

    return true;
}

void Controller::editRootNode()
{
    ModelRootNode* root = dynamic_cast<ModelRootNode*>(_modelNode);
    bool ok;
    //this->_view->createRootNodeGui(root->getName(),  root->getKey(), root->getIP(), root->getPort().toUShort(&ok));
}

void Controller::editConfig()
{
    //ModelConfig* conf = dynamic_cast<ModelConfig*>(_modelConfig);
    //this->_view->createConfigGui();
}

bool	Controller::changeRootNode()
{
    BodyConnexion* body = dynamic_cast<BodyConnexion*>(_view->getBody());

    bool ok;
    QString name = body->getName();
    QString ip = body->getAdress();
    quint16 port =  body->getPort().toUShort(&ok);
    QString pubkey = body->getKey();

    if (ok == false)
    {
        error("Error: Port is not a number");
        return false;
    }
    if (ip == "" || name == "" || pubkey == "")
    {
        error("one of the mandatory fields is missing");
        return false;
    }
    if (checkName(name) == false)
    {
        error("the name is not correctly formated (only alpha-numeric and '_' characters are allowed)");
        return false;
    }
    if (checkIP(ip) == false)
    {
        error("the IP is not correctly formated");
        return false;
    }

    try
    {
        dynamic_cast<ModelConnexion*>(this->_modelConnexion)->changeConnexionInfo(name, pubkey, ip, body->getPort());
        if (this->_network.isConnected() == false)
            this->_network.setConnection(ip, port);
    }
    catch (Exception* e)
    {
        error(e->getMessage());
        delete e;
        return false;
    }
    return true;
}

void		Controller::shutdown()
{
    _network.shutdown();
}

void		Controller::restart()
{
    try
    {
        ModelRootNode* node = dynamic_cast<ModelRootNode*>(this->_modelNode);
        bool ok;
        quint16 port = node->getPort().toUShort(&ok);
        this->_network.resetConnection(node->getIP(), port);
    }
    catch (Exception* e)
    {
        error(e->getMessage());
        delete e;
    }
}

const QMap<QString, QVariant>* Controller::getConfigMenu() const
{
    return _modelConfig->getData();
}

void Controller::changeConfig()
{
  //TODO : get QMAP<QString, QVariant>* changes = _view->getChangesInConfig();
  //if (changes != 0)
  // {
  //   for every change in changes {
  //     if exists in modelconfig
  //        write change in modelConfig
  //   }
  // }
  //  this->_view->deleteConfig();
}

void    Controller::setConnexionParam()
{
    BodyConnexion* body = dynamic_cast<BodyConnexion*>(_view->getBody());
    const QMap<QString, QVariant>* info = dynamic_cast<ModelConnexion*>(_modelConnexion)->getData();
    QVariant v = info->operator []("Name");
    body->setName(v.toString());
    v = info->operator[]("Adress");
    body->setAdress(v.toString());
    v = info->operator[]("Port");
    body->setPort(v.toString());
    v = info->operator[]("Key");
    body->setKey(v.toString());
}

void    Controller::initWindow()
{
    setConnexionParam();
}

bool    Controller::checkIPv4(QString& str) const
{
    QStringList elements = str.split(".");

    if (elements.size() != 4)
        return false;
    for (int i = 0; i < 4; ++i)
    {
        bool ok = true;
        int nb = elements[i].toInt(&ok);
        if (ok == false || nb < 0 || nb > 255)
            return false;
    }
    return true;
}

bool    Controller::checkIPv6(QString& str) const
{
    QStringList elements = str.split(":");

    if (elements.size() != 6)
        return false;
    for (int i = 0; i < 6; ++i)
    {
      // if (elements[i].toUInt() < 0 || elements[i].toUInt() > 255)
      //   return false;
    }
    return true;
}

bool    Controller::checkIP(QString& str) const
{
    if (checkIPv4(str))
        return true;
    if (checkIPv6(str))
        return true;
    if (checkHostNameFormat(str))
        return true;
    return false;
}

bool    Controller::checkName(QString& str) const
{
    QRegExp rx("^[a-zA-Z0-9_]+$");
    return str.contains(rx);
}

bool    Controller::checkHostNameFormat(QString& str) const
{
    QRegExp rx("^[a-zA-Z][a-zA-Z\\-\\.0-9]*[a-zA-Z]$");
    return str.contains(rx);
}

QString	Controller::openPubKey()
{
    //TODO : make this part more secure and check size/validity
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open File"));

    QString key = "";
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return key;

    while (!file.atEnd()) {
        key += file.readLine();
    }

    return key;
}

void Controller::connected()
{
    this->_view->connected();
}

void Controller::disconnected()
{
    this->_view->disconnected();
    dynamic_cast<ModelContact*>(this->_modelContacts)->clear();
}

void Controller::writeToSocket(const QString& buff)
{
    _network.write(buff);
}

void Controller::printError(const QString& message)
{
    error(message);
}
