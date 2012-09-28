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
  _commands["EditContact"] = &ModelContact::editContact;
}

void  ModelContact::print()
{
}

void  ModelContact::feedData(const QString& command, const QVariant& data)
{
    if (_commands.contains(command) == true)
    {
        QMap<QString, QVariant> person = data.toMap();
        QString key = "";
        QVector<QString> v;

        if (person.contains("Name") == true)
            v.append(person["Name"].toString());
        if (person.contains("Key") == true)
	    v.append(person["Key"].toString());
        if (person.contains("Ip") == true)
            v.append(person["Ip"].toString());
        if (person.contains("Old") == true)
            v.append(person["Old"].toString());
        (this->*_commands[command])(v);
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
    if (param.size() < 3)
        throw new Exception("Error: missing parameters to add contact");
    const QString &name = param[0];
    const QString &key = param[1];
    const QString &ip = param[2];

    if (name.length() == 0)
        throw new Exception("Error: No name");
    if (_contacts.contains(name))
        throw new Exception("Error: Name already exist");
    QMap<QString, QVariant> tmp;
    tmp.insert("Key", QVariant(key));
    tmp.insert("Ip", QVariant(ip));
    _contacts[name] = QVariant(tmp);
    _view->addContact(name);
}

const QString ModelContact::getKey(const QString &name)
{
    if (_contacts.contains(name) == 0)
        throw new Exception("Error: Name does not exist");
    return (_contacts[name].toMap())["Key"].toString();
}

const QString ModelContact::getIp(const QString &name)
{
    if (_contacts.contains(name) == 0)
        throw new Exception("Error: Name does not exist");
    return (_contacts[name].toMap())["Ip"].toString();
}

void ModelContact::delContact(const QVector<QString>& param)
{
    if (param.size() < 1)
        throw new Exception("Error: missing parameter to delete a contact");
    const QString& name = param[0];
    if (_contacts.remove(name) != 1)
      throw new Exception("Error: Name (" + name + ") does not exist");
    this->_view->deleteNamed(name);
}

void  ModelContact::editContact(const QVector<QString>& param)
{
    if (param.size() < 4)
        throw new Exception("Error: missing parameters to edit contact");
    const QString& name = param[0];
    const QString& key  = param[1];
    const QString& ip   = param[2];
    const QString& old  = param[3];

    QVector<QString> v;
    v.append(old);
    this->delContact(v);
    v.remove(0);
    v.append(name);
    v.append(key);
    v.append(ip);
    this->addContact(v);
}

void ModelContact::clear()
{
  _contacts.clear();
}
