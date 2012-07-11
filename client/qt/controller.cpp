#include <QDebug>
#include <QRegExp>
#include "iclientgui.h"
#include "controller.h"
#include "contact.h"
#include "exception.h"
#include "imodel.h"

QMap<QString, QString> Controller::_correspondence = QMap<QString, QString>();

Controller::Controller(IClientGUI*  gui) :
  _view(gui),
  _network(*this),
  _models()
{
  _modelContacts = new ModelContact(*this, gui);
  _modelNode = new ModelRootNode(*this);
  _modelLog = new ModelLog(*this);
  _modelConfig = new ModelConfig(*this);

  _models[_modelContacts->getObjectName()] = _modelContacts;
  _models[_modelNode->getObjectName()] = _modelNode;
  _models[_modelLog->getObjectName()] = _modelLog;
  _models[_modelConfig->getObjectName()] = _modelConfig;

  _correspondence["AddContact"] = "Contact";
}

void Controller::feedData(const QVariant& data)
{
  QMap<QString, QVariant> map = data.toMap();
  QMap<QString, QVariant>::const_iterator it(map.begin());
  const QMap<QString, QVariant>::const_iterator it_end = map.end();
  for (; it != it_end; ++it)
    {
      QString commande = it.key();
      try {
	_models[_correspondence[commande]]->feedData(commande, it.value());
      }
      catch (Exception *e)
	{
	  this->_view->printError(e->getMessage());
	  delete e;
	}
    }
}
void Controller::appendLog(const QString &s)
{
  this->_view->appendLog(s);
}

void Controller::editContact(QListWidgetItem *item)
{
  try {
    this->_view->createAddContact(item->text(), dynamic_cast<ModelContact*>(_modelContacts)->getKey(item->text()));
  }
  catch (Exception *e)
    {
      this->_view->printError(e->getMessage());
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
  this->_view->printError(s);
}

void Controller::deleteContact()
{
  try
    {
      QVector<QString> v;
      v.append(_view->getSelected());
      dynamic_cast<ModelContact*>(this->_modelContacts)->delContact(v);
      this->_view->deleteSelected();
    }
  catch (Exception *e)
    {
      this->_view->printError(e->getMessage());
      delete e;
    }
}

bool Controller::addContact()
{
  QString pubkey = this->_view->getNewContactKey();
  QString name = this->_view->getNewContactName();

  if (name == "" || pubkey == "")
    {
      this->_view->printError("One of the mandatory fields is missing");
      return false;
    }
  if (checkName(name) == false)
    {
      this->_view->printError("the name is not correctly formated (only alpha-numeric and '_' characters are allowed)");
      return false;
    }

  if (!_view->getInitialContactName().isEmpty()) // TODO if user select a different row after editing
    this->deleteContact();
  try
    {
      QVector<QString> v;
      v.append(name);
      v.append(pubkey);
      dynamic_cast<ModelContact*>(this->_modelContacts)->addContact(v);
      writeToSocket(name);
    }
 catch (Exception *e)
   {
     this->_view->printError(e->getMessage());
     delete e;
     return false;
   }
  this->_view->deleteAddContact();

  return true;
  // new Contact (this->e_view->getNewContactName(),
  //              this->_view->getNewContactKey());
  //TODO integrate Contact in to model, validate DATA
}

void Controller::editRootNode()
{
  ModelRootNode* root = dynamic_cast<ModelRootNode*>(_modelNode);
  bool ok;
  this->_view->createRootNodeGui(root->getName(),  root->getKey(), root->getIP(), root->getPort().toUShort(&ok));
}

void Controller::editConfig()
{
  ModelConfig* conf = dynamic_cast<ModelConfig*>(_modelConfig);
  this->_view->createConfigGui();
}

bool	Controller::changeRootNode()
{
  bool ok;
  quint16 port =  _view->getRootPort().toUShort(&ok);
  QString ip = this->_view->getRootIP();
  QString pubkey = this->_view->getRootKey();
  QString name = this->_view->getRootName();

  if (ok == false)
    {
	this->_view->printError("Error: Port is not a number");
	return false;
    }
  if (ip == "" || name == "" || pubkey == "")
    {
	this->_view->printError("one of the mandatory fields is missing");
	return false;
    }
  if (checkName(name) == false)
    {
	this->_view->printError("the name is not correctly formated (only alpha-numeric and '_' characters are allowed)");
	return false;
    }
  if (checkIP(ip) == false)
    {
	this->_view->printError("the IP is not correctly formated");
	return false;
    }

  try
    {
	dynamic_cast<ModelRootNode*>(this->_modelNode)->changeRootNode(name, pubkey, ip, _view->getRootPort());
	if (this->_network.isConnected() == false)
	  this->_network.setConnection(ip, port);
    }
  catch (Exception* e)
    {
	this->_view->printError(e->getMessage());
	delete e;
	return false;
    }
  this->_view->deleteRootNode();
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
	this->_view->printError(e->getMessage());
	delete e;
    }
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
  this->_view->deleteConfig();
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
  return false;
}

bool    Controller::checkName(QString& str) const
{
  QRegExp rx("^[a-zA-Z0-9_]+$");
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
}

void Controller::writeToSocket(const QString& buff)
{
  _network.write(buff);
}
