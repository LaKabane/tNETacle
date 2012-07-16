#ifndef MODELCONTACT_H_
# define MODELCONTACT_H_

#include <QMap>
#include <QString>
#include "imodel.h"
#include "iclientgui.h"

class Controller;

class ModelContact : public IModel
{
  typedef void (ModelContact::*fun)(const QVector<QString>&);
  typedef QMap<QString, fun> mapfun;

public:
  ModelContact(Controller &, IClientGUI*);
  virtual       ~ModelContact(){}

  void          addContact(const QVector<QString>&);
  const QString getKey(const QString &);
  void          delContact(const QVector<QString>&);
  void          editContact(const QVector<QString>&);
  void	print();
  void	clear();

  virtual void  feedData(const QString &,const QVariant&);
  virtual const QString &getObjectName() const;

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
