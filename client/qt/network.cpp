/*
 * Copyright (c) 2012 Florent Tribouilloy <tribou_f AT epitech DOT net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "network.h"
#include "controller.h"
#include "tclt_parse.h"
#include "exception.h"
#include <QBuffer>

Network::Network(Controller& controller)
  : _socket(),
    _controller(controller),
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

    try
    {
        device.buffer().append('\0');
        if (tclt_dispatch_command(device.buffer().constData(), 0))
            throw new Exception("Error : command not found\n");
        _socket.read(data.size());
    }
    catch (Exception* e)
    {
        this->_controller.printError(e->getMessage());
        delete e;
    }
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
