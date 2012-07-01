#ifndef CONTROLLER_H_
# define CONTROLLER_H_

#include <QMap>
#include <QObject>
#include <QListWidgetItem>
#include "model_contacts.h"
#include "network.h"

class ClientGUI;

class Controller : public QObject
{
  Q_OBJECT
public:
  Controller(ClientGUI &);
  const QString &getIp() const;
  quint16 getPort() const;
public slots:
  void addContact();
  void deleteContact();
  void editContact(QListWidgetItem *);

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
};


#endif /* !CONTROLLER_H_ */
