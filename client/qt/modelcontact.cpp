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
  _commands["DeleteContact"] = &ModelContact::delContact;
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
	  QVector<QString> v;
	  v.append(it_p.key());
	  if (person.contains("Name") == true)
	    v[0] = person["Name"].toString();
	  if (person.contains("Key") == true)
	    v.append(person["Key"].toString());
	  if (person.contains("Old") == true)
	    v.append(person["Old"].toString());
	  (this->*_commands[command])(v);
	}
    }
  else
    throw new Exception("Error: command does not exist!");
}

const QString& ModelContact::getObjectName() const
{
  return ModelContact::_name;
}

const QMap<QString, QVariant>* ModelContact::getData() const
{
  return &_contacts;
}

void ModelContact::addContact(const QVector<QString>& param)
{
  if (param.size() < 2)
    throw new Exception("Error: missing parameters to add contact");
  const QString &name = param[0];
  const QString &key = param[1];
  if (!name.length())
    throw new Exception("Error: No name");
  if (_contacts.contains(name))
    throw new Exception("Error: Name already exist");
  _contacts[name] = key;
  _view->addContact(name);
}

const QString ModelContact::getKey(const QString &name)
{
  if (_contacts.contains(name) == 0)
    throw new Exception("Error: Name does not exist");
  return _contacts[name].toString();
}

void ModelContact::delContact(const QVector<QString>& param)
{
  if (param.size() < 1)
    throw new Exception("Error: missing parameter to delete a contact");
  const QString& name = param[0];
  if (_contacts.remove(name) != 1)
    throw new Exception("Error: Name does not exist");
  this->_view->deleteNamed(name);
}

void  ModelContact::editContact(const QVector<QString>& param)
{
  if (param.size() < 3)
    throw new Exception("Error: missing parameters to edit contact");
  const QString& old = param[0];
  const QString& name = param[1];
  const QString& key = param[2];

  QVector<QString>v;
  v.append(old);
  this->delContact(v);
  v.remove(0);
  v.append(name);
  v.append(key);
  this->addContact(v);
}
