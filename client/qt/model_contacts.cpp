#include "model_contacts.h"


ModelContacts::ModelContacts(Controller &controller):
  _contacts(),
  _controller(controller)
{

}


void ModelContacts::addContact(const QString &name, const QString &key)
{
  if (!name.length())
    ; // TODO: throw
  if (_contacts.contains(name))
    ; // TODO: verify
   _contacts[name] = key;
}

const QString &ModelContacts::getKey(const QString &name)
{
  if (!_contacts.contains(name))
    ;//TODO: throw
  return _contacts[name];
}

void ModelContacts::delContact(const QString &name)
{
  if (_contacts.remove(name) != 1)
    ;//TODO: throw
}

void  ModelContacts::editContact(const QString &old, const QString &name, const QString &key)
{
  this->delContact(old);
  this->addContact(name, key);
}
