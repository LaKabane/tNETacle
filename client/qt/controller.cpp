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
#include "iclient.h"
#include "controller.h"
#include "protocol.h"
#include "exception.h"
#include "imodel.h"
#include "qclient.h"
#include "bodyconnexion.h"
#include "bodymain.h"
#include "bodyaddcontact.h"
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
    (void)data;
}

int
Controller::add_peer_controll(void *f, void *internal)
{
    peer *p = static_cast<peer*>(f);
    (void)internal;

    BodyMain* view = dynamic_cast<BodyMain*>(BodyMain::get(0, 0));
    if(view != 0)
    {
        view->addNewPeer(p->name);
    }
    return 0;
}

int
Controller::add_log_controll(void *f, void *internal)
{
    char *str = static_cast<char*>(f);
    (void)internal;

    std::cout << "add log :" << str << std::endl;
    return 0;
}

int
Controller::delete_peer_controll(void *f, void *internal)
{
    char *str = static_cast<char*>(f);
    QString str2(str);
    (void)internal;
    qDebug() << str2;

    return 0;
}

int
Controller::edit_peer_controll(void *f, void*internal)
{
    (void)f;
    (void)internal;
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
    (void)s;
    //this->_view->appendLog(s);
}

void Controller::editContact(QListWidgetItem* item)
{
    (void)item;
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
    (void)s;
    //this->_view->printError(s);
}

void Controller::deleteContact(const QString& name)
{
    try
    {
        this->writeToSocket(tclt_delete_peer(name.toStdString().c_str()));
    }
    catch (Exception *e)
    {
        error(e->getMessage());
	delete e;
    }
}

bool Controller::addContact()
{
    BodyAddContact* view = dynamic_cast<BodyAddContact*>(_view->getBody());
    if (view == 0)
        return false;

    const QString& pubkey = view->getNewContactKey();
    const QString& name = view->getNewContactName();
    const QString& ip = view->getContactIp();

    if (name == "" || pubkey == "" || ip == "")
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

    try
    {
        peer p;
        p.name = strdup(name.toStdString().c_str());
        p.ip = strdup(ip.toStdString().c_str());
        p.key = strdup(pubkey.toStdString().c_str());
        if (p.name == 0 || p.ip == 0 || p.key == 0)
            throw new Exception("Impossible to duplicate elements");
        dynamic_cast<ModelContact*>(this->_modelContacts)->addContact(&p);
        this->writeToSocket(tclt_add_peer(&p));
        free(p.name);
        free(p.ip);
        free(p.key);
        view->cleanField();
    }
    catch (Exception *e)
    {
        error(e->getMessage());
        delete e;
        return false;
    }
    QClient* client = dynamic_cast<QClient*>(_view);
    if (client == 0)
        return false;
    client->changePrevBody();

    return true;
}

void Controller::editRootNode()
{
    //ModelRootNode* root = dynamic_cast<ModelRootNode*>(_modelNode);
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

bool    Controller::checkIPv4(const QString& str) const
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

bool    Controller::checkIPv6(const QString& str) const
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

bool    Controller::checkIP(const QString &str) const
{
    if (checkIPv4(str))
        return true;
    if (checkIPv6(str))
        return true;
    if (checkHostNameFormat(str))
        return true;
    return false;
}

bool    Controller::checkName(const QString& str) const
{
    QRegExp rx("^[a-zA-Z0-9_]+$");
    return str.contains(rx);
}

bool    Controller::checkHostNameFormat(const QString &str) const
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

void Controller::viewAddContact()
{
    QClient* client = dynamic_cast<QClient*>(_view);
    if (client == 0)
        return ;
    client->changeNextBody(QClient::ADDCONTACT);
}

void Controller::unuseAddContact()
{
    QClient* client = dynamic_cast<QClient*>(_view);
    if (client == 0)
        return ;
    client->changePrevBody();
}
