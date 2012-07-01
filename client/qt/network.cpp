#include "network.h"
#include "Controller.h"


Network::Network(Controller &controller, const QString &ip, const quint16 port)
  : _socket(),
    _ip(ip),
    _port(port),
    _controller(controller)
{
  QObject::connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
  _socket.connectToHost(_ip, port);
}


void Network::setConnection(const QString &ip, const quint16 port) // we want to set BOTH!
{
  _ip = ip;
  _port = port;
  _socket.close();
  _socket.connectToHost(_ip, port);
}

void Network::error(QAbstractSocket::SocketError)
{
  _controller.error(_socket.errorString());
}


const QString &Network::getIp() const
{
  return _ip;
}

quint16       Network::getPort() const
{
  return _port;
}
