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
  Network(Controller &controller, const QString &ip = "127.0.0.1", const quint16 port = 4243);
  virtual ~Network() {}
  void setConnection(const QString &, const quint16); // we want to set BOTH!
  const QString &getIp() const;
  quint16       getPort() const;
public slots:
  void error(QAbstractSocket::SocketError);
private slots:
  void read();
private:
  QTcpSocket    _socket;
  QString       _ip;// can also be domain
  quint16       _port;
  Controller    &_controller;
  QJson::Parser _parser;
};


#endif /* !NETWORK_H_ */
