#include <QDebug>
#include "clientgui.h"
#include "controller.h"
#include "contact.h"
#include "exception.h"
#include "imodel.h"

Controller::Controller(ClientGUI & gui) :
  _view(gui),
  _network(*this),
  _models()
{
  _modelContacts = new ModelContact(*this);
  _modelLog = new ModelLog(*this);

  _models.append(_modelContacts);
  //  _models.append(_modelNode);
  _models.append(_modelLog);
}

void Controller::feedData(const QVariant &data)
{
  qDebug() << _models.size();
  QMap<QString, QVariant> map = data.toMap();
  QMap<QString, QVariant>::const_iterator it(map.begin());
  for (const QMap<QString, QVariant>::const_iterator it_end = map.end(); it != it_end; ++it)
    {
      QString commande = it.key().left(3);
      QString object = it.key().right(it.key().length() - 3);
      QMap<QString, QVariant > data = it.value().toMap();
      int end = _models.size();
      qDebug() << end;
      for (int i = 0; i < end ; ++i)
        {
          qDebug() << _models[i]->getObjectName();
        if (_models[i]->getObjectName() == object)
          {
            _models[i]->feedData(commande, data);
            break;
          }
        }
    }
}
void Controller::appendLog(const QString &s)
{
  this->_view.appendLog(s);
}

void Controller::editContact(QListWidgetItem *item)
{
  try {
    this->_view.createAddContact(item->text(), dynamic_cast<ModelContact*>(_modelContacts)->getKey(item->text()));
  }
  catch (Exception *e)
    {
      this->_view.printError(e->getMessage());
      delete e;
    }
}

const QString &Controller::getIp() const
{
  return (_network.getIp());
}

quint16 Controller::getPort() const
{
  return _network.getPort();
}

void Controller::error(const QString &s)
{
  this->_view.printError(s);
}

void Controller::deleteContact()
{
  try
    {
      dynamic_cast<ModelContact*>(this->_modelContacts)->delContact(_view.getSelected());
      this->_view.deleteSelected();
    }
  catch (Exception *e)
    {
      this->_view.printError(e->getMessage());
      delete e;
    }
}

void Controller::addContact()
{

  if (!_view.getInitialContactName().isEmpty()) // TODO if user select a different row after editing
    this->deleteContact();
  try
    {
      dynamic_cast<ModelContact*>(this->_modelContacts)->addContact(this->_view.getNewContactName(),
                                       this->_view.getNewContactKey());
      this->_view.addContact(this->_view.getNewContactName());
    }
 catch (Exception *e)
   {
     this->_view.printError(e->getMessage());
     delete e;
   }
  this->_view.deleteAddContact();

  // new Contact (this->e_view.getNewContactName(),
  //              this->_view.getNewContactKey());
  //TODO integrate Contact in to model, validate DATA
}

void Controller::editRootNode()
{
  // TODO
  this->_view.createRootNodeGui(/* _rootNodeName */"",  /*_rootNodePubkey*/"", _network.getIp(), _network.getPort());
}

void	Controller::changeRootNode()
{
  // TODO QSTRing
  // _rootNodeName = this->_view.getRootName();
  bool ok;
  quint16 port =  _view.getRootPort().toUShort(&ok);
  try
    {
      if (ok)
        this->_network.setConnection(_view.getRootIP(), port);
      else
        this->_view.printError("Error: Port is not a number");
    }
  catch (Exception *e)
    {
      this->_view.printError(e->getMessage());
      delete e;
    }
  this->_view.deleteRootNode();
}
