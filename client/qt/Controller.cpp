#include "clientgui.h"
#include "controller.h"
#include "contact.h"

Controller::Controller(ClientGUI & gui) :
  _view(gui),
  _model()
{

}


void Controller::deleteContact()
{
  this->_model.remove(this->_view.getSelected());
  this->_view.deleteSelected();
}

void Controller::addContact()
{
  this->_view.addContact(this->_view.getNewContactName());
  this->_model[this->_view.getNewContactName()] = this->_view.getNewContactKey();
  this->_view.deleteAddContact();
  // new Contact (this->e_view.getNewContactName(),
  //              this->_view.getNewContactKey());
  //TODO integrate Contact in to model, validate DATA


}

