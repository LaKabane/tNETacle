#ifndef NETWORK_H_
# define NETWORK_H_

#include <QString>
#include <QTcpSocket>

class Network
{
public:
  Network(const QString &ip = "127.0.0.1", const quint16 port = 4243);
  virtual ~Network(){}
  void setConnection(const QString &, const quint16); // we want to set BOTH!
  const QString &getIp() const;
  quint16       getPort() const;
private:
  QTcpSocket    _socket;
  QString       _ip;// can also be URL
  quint16       _port;
};


#endif /* !NETWORK_H_ */
