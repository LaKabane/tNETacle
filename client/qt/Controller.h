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
private:
  ClientGUI &_view;
};


#endif /* !CONTROLLER_H_ */
