#include <QMap>
#include <QObject>


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
private:
  ClientGUI &_view;
  QMap<QString, QString> _model; // This model is temporary, waiting for the real one to be ready.
};


#endif /* !CONTROLLER_H_ */
