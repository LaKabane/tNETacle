#ifndef MODEL_CONTACT_H_
# define MODEL_CONTACT_H_

#include <QMap>
#include <QString>
#include "imodel.h"

class Controller;

class ModelContact : public IModel
{
 public:
  ModelContact(Controller &);
  void          addContact(const QString&, const QString&);
  const QString &getKey(const QString &);
  void          delContact(const QString &);
  void          editContact(const QString&, const QString&, const QString&);
  virtual       ~ModelContact(){};
  virtual void  feedData(const QString &,const QMap<QString, QVariant> &);
private:
  QMap<QString, QString> _contacts;
  Controller    &_controller;
  static const QString _name;
protected: // from IModel
  virtual const QString &getObjectName() const;
  virtual const QMap<QString, QString> &getData() const;

};

#endif /* !MODEL_CONTACT_H */
