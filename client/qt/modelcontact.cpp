#include <QDebug>
#include "exception.h"
#include "modelcontact.h"

const QString ModelContact::_name = "Contacts";

ModelContact::ModelContact(Controller &controller):
  _contacts(),
  _controller(controller)
{

}

void  ModelContact::feedData(const QString &, const QMap<QString, QVariant> &)
{

}

const QString &ModelContact::getObjectName() const
{
  return ModelContact::_name;
}

const QMap<QString, QString> &ModelContact::getData() const
{
  return _contacts;
}

void ModelContact::addContact(const QString &name, const QString &key)
{
  if (!name.length())
    throw new Exception("Error: No name");
  if (_contacts.contains(name))
    throw new Exception("Error: Name already exist");
   _contacts[name] = key;
   qDebug() << this->toJson();
}

const QString &ModelContact::getKey(const QString &name)
{
  if (!_contacts.contains(name))
    throw new Exception("Error: Name does not exist");
  return _contacts[name];
}

void ModelContact::delContact(const QString &name)
{
  if (_contacts.remove(name) != 1)
    throw new Exception("Error: Name does not exist");
}

void  ModelContact::editContact(const QString &old, const QString &name, const QString &key)
{
  this->delContact(old);
  this->addContact(name, key);
}
