#include "clientgui.h"
#include "Controller.h"
#include "contact.h"
#include "exception.h"

Controller::Controller(ClientGUI & gui) :
  _view(gui),
  _model_contacts(*this),
  _network(*this)
{

}

void Controller::editContact(QListWidgetItem *item)
{
  try {
  this->_view.createAddContact(item->text(), _model_contacts.getKey(item->text()));
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
      this->_model_contacts.delContact(_view.getSelected());
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
      this->_model_contacts.addContact(this->_view.getNewContactName(),
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
  this->_view.createRootNodeGui(_rootNodeName,  _rootNodePubkey, _network.getIp(), _network.getPort()
);
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
