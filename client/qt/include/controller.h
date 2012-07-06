#ifndef CONTROLLER_H_
# define CONTROLLER_H_

#include <QMap>
#include <QObject>
#include <QVariant>
#include <QListWidgetItem>
#include "modelcontacts.h"
#include "modellog.h"
#include "network.h"

class ClientGUI;

class Controller : public QObject
{
  Q_OBJECT
public:
  Controller(ClientGUI &);
  const QString &getIp() const;
  quint16 getPort() const;
  void feedData(const QVariant &);
  void error(const QString &);

public slots:
  void addContact();
  void deleteContact();
  void editContact(QListWidgetItem *);
  void appendLog(const QString &);
  void editRootNode();
  void changeRootNode();
private:
  ClientGUI &_view;
  ModelContacts _model_contacts;
  bool  editing;

  // // TODO : put the root node here with a correct type
  // REMOVED: in _network.
  QString _rootNodeName;
  // QString _rootNodeIP;
  QString _rootNodePubkey;
  Network _network;
  ModelLog _modelLog;
  QVector<Model *> _models;
};


#endif /* !CONTROLLER_H_ */
