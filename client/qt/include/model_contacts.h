#ifndef MODEL_CONTACTS_H_
# define MODEL_CONTACTS_H_

#include <QMap>
#include <QString>
#include "IModel.h"

class Controller;

class ModelContacts : public IModel
{
 public:
  ModelContacts(Controller &);
  void          addContact(const QString&, const QString&);
  const QString &getKey(const QString &);
  void          delContact(const QString &);
  void          editContact(const QString&, const QString&, const QString&);
  virtual       ~ModelContacts(){};
private:
  QMap<QString, QString> _contacts;
  Controller    &_controller;
};

#endif /* !MODEL_CONTACTS_H */
