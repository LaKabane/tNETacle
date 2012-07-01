#include "network.h"

Network::Network(const QString &ip, const quint16 port)
  : _socket(),
    _ip(ip),
    _port(port)
{
    _socket.connectToHost(_ip, port);
}

void Network::setConnection(const QString &ip, const quint16 port) // we want to set BOTH!
{
  _ip = ip;
  _port = port;
  _socket.close();
  _socket.connectToHost(_ip, port);
}

const QString &Network::getIp() const
{
  return _ip;
}

quint16       Network::getPort() const
{
  return _port;
}
