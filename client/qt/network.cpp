#include "network.h"
#include "controller.h"
#include <QBuffer>

Network::Network(Controller& controller)
  : _socket(),
    _controller(controller),
    _parser(),
    _isConnected(false)
{
  QObject::connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
  QObject::connect(&_socket, SIGNAL(readyRead()), this, SLOT(read()));
  QObject::connect(&_socket, SIGNAL(connected()), this, SLOT(connected()));
  QObject::connect(&_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}


void Network::read()
{
  if (_isConnected == false)
    return ;

  QBuffer device;
  device.open(QIODevice::ReadWrite);
  QByteArray data = _socket.peek(_socket.size());
  device.write(data);
  device.close();
   //QVariant var = _parser.parse(&_socket, &ok);

  bool ok;
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

void Network::write(const QString& buff)
{
  if (_isConnected == false)
    return ;
  _socket.write(buff.toAscii().data(), buff.size());
  _socket.flush();
  _socket.waitForBytesWritten(buff.size());
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

void Network::disconnected()
{
  _isConnected = false;
  _controller.disconnected();
}

void Network::connected()
{
  _isConnected = true;
  _controller.connected();
}

bool Network::isConnected() const
{
  return _isConnected;
}
