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

#ifndef NETWORK_H_
# define NETWORK_H_

#include <QString>
#include <QSslSocket>

class Controller;

class Network : public QObject
{
  Q_OBJECT
public:
  Network(Controller& controller);
  virtual	~Network() {}

  void		setConnection(const QString &, const quint16); // we want to set BOTH!
  void		resetConnection(const QString &, const quint16);
  void		shutdown();
  bool		isConnected() const;
  void		write(const QString&);
  
public slots:
  void		error(QAbstractSocket::SocketError);

private slots:
  void		read();
  void		connected();
  void		disconnected();

private:
  QSslSocket	_socket;
  Controller&	_controller;
  bool			_isConnected;
};

#endif /* !NETWORK_H_ */
