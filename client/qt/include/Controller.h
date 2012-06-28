#include <QMap>
#include <QObject>
#include <QListWidgetItem>
#include "model_contacts.h"
#ifndef CONTROLLER_H_
# define CONTROLLER_H_


class ClientGUI;

class Controller : public QObject
{
  Q_OBJECT
public:
  Controller(ClientGUI &);
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

  // TODO : put the root node here with a correct type
  QString _rootNodeName;
  QString _rootNodeIP;
  QString _rootNodePubkey;
};


#endif /* !CONTROLLER_H_ */
