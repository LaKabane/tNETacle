#include "clientgui.h"
#include "controller.h"
#include "contact.h"

Controller::Controller(ClientGUI & gui) :
  _view(gui),
  _model()
{

}

void Controller::edit(QListWidgetItem *item)
{
  this->_view.createAddContact(item->text(), _model[item->text()]);
}

void Controller::deleteContact()
{
  this->_model.remove(this->_view.getSelected());
  this->_view.deleteSelected();
}

void Controller::addContact()
{
  if (!_view.getInitialContactName().isEmpty()) // TODO if user select a different row after editing
    this->deleteContact();
  this->_view.addContact(this->_view.getNewContactName());
  this->_model[this->_view.getNewContactName()] = this->_view.getNewContactKey();
  this->_view.deleteAddContact();
  // new Contact (this->e_view.getNewContactName(),
  //              this->_view.getNewContactKey());
  //TODO integrate Contact in to model, validate DATA


}

