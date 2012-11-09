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

#ifndef CONTROLLER_H_
# define CONTROLLER_H_

#include <QMap>
#include <QObject>
#include <QVariant>
#include <QListWidgetItem>
#include <QFileDialog>
#include "modelcontact.h"
#include "modelconfig.h"
#include "modellog.h"
#include "modelrootnode.h"
#include "modelconnexion.h"
#include "network.h"

class IClient;

class Controller : public QObject
{
  Q_OBJECT
public:
  Controller();
  virtual ~Controller() {}

  const QString		getIp() const;
  quint16		getPort() const;
  void			feedData(const QVariant &);
  void			error(const QString &);
  QString		openPubKey();

  void			shutdown();
  void			restart();

  void			connected();
  void			disconnected();
  void			writeToSocket(const QString& buff);

  const QMap<QString, QVariant>* getConfigMenu() const;

  void			printError(const QString&);
  void          setGui(IClient* gui) {_view = gui;}
  void          setConnexionParam();

  void          initWindow();

public slots:
  bool			addContact();
  void			deleteContact();
  void			editContact(QListWidgetItem *);
  void			appendLog(const QString &);
  void			editRootNode();
  void			editConfig();
  bool			changeRootNode();

  void			changeConfig();

  bool			checkIP(const QString&) const;
  bool			checkName(const QString&) const;
  void          viewAddContact();
  void          unuseAddContact();

private:
  bool			checkIPv4(const QString &) const;
  bool			checkIPv6(const QString&) const;
  bool			checkHostNameFormat(const QString& str) const;
  void          init_callback();
  static int           add_peer_controll(void *f);
  static int           delete_peer_controll(void *f);
  static int           edit_peer_controll(void *f);
  static int           add_log_controll(void *f);

  IClient*    _view;
  bool          editing;
  IModel*		_modelContacts;
  IModel*		_modelNode;
  IModel*		_modelLog;
  IModel*		_modelConfig;
  IModel*       _modelConnexion;

  // // TODO : put the root node here with a correct type
  // REMOVED: in _network.
  Network		_network;

  QMap<QString, IModel*>	_models;

};


#endif /* !CONTROLLER_H_ */
