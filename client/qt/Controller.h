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
  void edit(QListWidgetItem *);
private:
  ClientGUI &_view;
  QMap<QString, QString> _model; // This model is temporary, waiting for the real one to be ready.
  bool  editing;
};


#endif /* !CONTROLLER_H_ */
