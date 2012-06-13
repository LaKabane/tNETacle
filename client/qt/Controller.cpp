#include "clientgui.h"
#include "controller.h"
#include "contact.h"

Controller::Controller(ClientGUI & gui) :
_view(gui)
{

}

void Controller::addContact()
{
  this->_view.addContact(this->_view.getNewContactName());
  this->_view.deleteAddContact();
  // new Contact (this->e_view.getNewContactName(),
  //              this->_view.getNewContactKey());
  //TODO integrate Contact in to model


}

