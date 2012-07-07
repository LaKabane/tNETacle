#ifndef NETWORK_H_
# define NETWORK_H_

#include <QString>
#include <QTcpSocket>
#include <qjson/parser.h>

class Controller;

class Network : public QObject
{
  Q_OBJECT
public:
  Network(Controller &controller);
  virtual ~Network() {}
  void setConnection(const QString &, const quint16); // we want to set BOTH!

public slots:
  void error(QAbstractSocket::SocketError);

private slots:
  void read();

private:
  QTcpSocket    _socket;
  Controller&   _controller;
  QJson::Parser _parser;
};

#endif /* !NETWORK_H_ */
