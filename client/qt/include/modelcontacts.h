#ifndef MODEL_CONTACTS_H_
# define MODEL_CONTACTS_H_

#include <QMap>
#include <QString>
#include "model.h"

class Controller;

class ModelContacts : public Model
{
 public:
  ModelContacts(Controller &);
  void          addContact(const QString&, const QString&);
  const QString &getKey(const QString &);
  void          delContact(const QString &);
  void          editContact(const QString&, const QString&, const QString&);
  virtual       ~ModelContacts(){};
  virtual void  feedData(const QString &,const QMap<QString, QVariant> &);
private:
  QMap<QString, QString> _contacts;
  Controller    &_controller;
  static const QString _name;
protected: // from IModel
  virtual const QString &getObjectName() const;
  virtual const QMap<QString, QString> &getData() const;

};

#endif /* !MODEL_CONTACTS_H */
