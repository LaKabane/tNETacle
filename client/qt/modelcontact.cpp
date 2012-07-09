#include <QDebug>
#include "exception.h"
#include "modelcontact.h"

const QString ModelContact::_name = "Contact";
ModelContact::mapfun ModelContact::_commands = mapfun();

ModelContact::ModelContact(Controller& controller, IClientGUI* gui):
  _contacts(),
  _controller(controller),
  _view(gui)
{
  _commands["AddContact"] = &ModelContact::addContact;
}

void  ModelContact::print()
{
}

void  ModelContact::feedData(const QString& command, const QVariant& data)
{
  if (_commands.contains(command) == true)
    {
      QMap<QString, QVariant> people = data.toMap();
      QMap<QString, QVariant>::const_iterator it_p(people.begin());
      const QMap<QString, QVariant>::const_iterator ite_p(people.end());
      for (; it_p != ite_p; ++it_p)
	{
	  QMap<QString, QVariant> person = (*it_p).toMap();
	  QString key = "";
	  if (person.contains("Key") == true)
	    key = person["Key"].toString();
	  (this->*_commands[command])(it_p.key(), key);
	}
    }
  else
    qDebug() << command << "does not exist";
}

const QString& ModelContact::getObjectName() const
{
  return ModelContact::_name;
}

const QMap<QString, QVariant>* ModelContact::getData() const
{
  return &_contacts;
}

void ModelContact::addContact(const QString &name, const QString &key)
{
  if (!name.length())
    throw new Exception("Error: No name");
  if (_contacts.contains(name))
    throw new Exception("Error: Name already exist");
  _contacts[name] = key;
  _view->addContact(name);
  //qDebug() << this->toJson();
}

const QString ModelContact::getKey(const QString &name)
{
  if (_contacts.contains(name) == 0)
    throw new Exception("Error: Name does not exist");
  return _contacts[name].toString();
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
