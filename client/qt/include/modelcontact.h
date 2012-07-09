#ifndef MODELCONTACT_H_
# define MODELCONTACT_H_

#include <QMap>
#include <QString>
#include "imodel.h"
#include "iclientgui.h"

class Controller;

class ModelContact : public IModel
{
  typedef void (ModelContact::*fun)(const QString&, const QString&);
  typedef QMap<QString, fun> mapfun;
 public:
  ModelContact(Controller &, IClientGUI*);
  void          addContact(const QString&, const QString&);
  const QString getKey(const QString &);
  void          delContact(const QString &);
  void          editContact(const QString&, const QString&, const QString&);
  virtual       ~ModelContact(){};
  virtual void  feedData(const QString &,const QVariant&);
  virtual const QString &getObjectName() const;
  void	print();

private:
  QMap<QString, QVariant> _contacts;
  Controller    &_controller;
  static const QString _name;
  static mapfun	_commands;
  IClientGUI*	_view;

protected: // from IModel
  virtual const QMap<QString, QVariant>* getData() const;

};

#endif /* !MODELCONTACT_H */
