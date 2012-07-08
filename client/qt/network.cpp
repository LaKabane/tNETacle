#include "network.h"
#include "controller.h"
#include <QBuffer>

Network::Network(Controller &controller)
  : _socket(),
    _controller(controller),
    _parser()
{
  QObject::connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
  QObject::connect(&_socket, SIGNAL(readyRead()), this, SLOT(read()));
}


void Network::read()
{
  bool ok;
  QBuffer device;
  device.open(QIODevice::ReadWrite);
  QByteArray data = _socket.peek(_socket.size());
  device.write(data);
  device.close();
   //QVariant var = _parser.parse(&_socket, &ok);
  QVariant var = _parser.parse(&device, &ok);
  if (!ok)
    {
      qDebug() << "error parsing";
      _controller.error(_parser.errorString());
      return ;
    }
  _socket.read(data.size());
  _controller.feedData(var);
}

void Network::setConnection(const QString& ip, const quint16 port) // we want to set BOTH!
{
  _socket.connectToHost(ip, port);
}

void Network::resetConnection(const QString& ip, const quint16 port) // we want to set BOTH!
{
  _socket.close();
  _socket.connectToHost(ip, port);
}

void Network::error(QAbstractSocket::SocketError)
{
  _controller.error(_socket.errorString());
}

void Network::shutdown()
{
  _socket.close();
}
