#include <QMap>
#include <QObject>
#include <QListWidgetItem>

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
  QMap<QString, QString> _model; // This model is temporary, waiting for the real one to be ready.
  bool  editing;

  // TODO : put the root node here with a correct type
  QString _rootNodeName;
  QString _rootNodeIP;
  QString _rootNodePubkey;
};


#endif /* !CONTROLLER_H_ */
