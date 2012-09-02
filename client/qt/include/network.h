#ifndef NETWORK_H_
# define NETWORK_H_

#include <QString>
#include <QTcpSocket>

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
  QTcpSocket    _socket;
  Controller&   _controller;
  bool		_isConnected;
};

#endif /* !NETWORK_H_ */
