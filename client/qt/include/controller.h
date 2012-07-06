#ifndef CONTROLLER_H_
# define CONTROLLER_H_

#include <QMap>
#include <QObject>
#include <QVariant>
#include <QListWidgetItem>
#include "modelcontact.h"
#include "modellog.h"
#include "network.h"

class IClientGUI;

class Controller : public QObject
{
  Q_OBJECT
public:
  Controller(IClientGUI*);
  virtual ~Controller() {}

  const QString&	getIp() const;
  quint16		getPort() const;
  void			feedData(const QVariant &);
  void			error(const QString &);

public slots:
  bool			addContact();
  void			deleteContact();
  void			editContact(QListWidgetItem *);
  void			appendLog(const QString &);
  void			editRootNode();
  bool			changeRootNode();

  bool			checkIP(QString&) const;
  bool			checkName(QString&) const;

private:
  bool			checkIPv4(QString&) const;
  bool			checkIPv6(QString&) const;

  IClientGUI*		_view;
  bool			editing;
  IModel*		_modelContacts;
  IModel*		_modelNode;
  IModel*		_modelLog;

  // // TODO : put the root node here with a correct type
  // REMOVED: in _network.
  Network		_network;

  QVector<IModel*>	_models;
};


#endif /* !CONTROLLER_H_ */
