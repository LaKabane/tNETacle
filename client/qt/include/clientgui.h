#ifndef CLIENTGUI_H
#define CLIENTGUI_H

#include <QMainWindow>
#include <QTimer>
#include "ui_client.h"
#include "controller.h"
#include "iclientgui.h"

namespace Ui {
    class ClientGUI;
}

class AddContactGui;
class RootNodeGui;
class ConfigGui;

class ClientGUI : public QMainWindow, public Ui::ClientGUI, public IClientGUI
{
    Q_OBJECT
public:
  explicit		ClientGUI(QMainWindow *parent = 0);
  virtual		~ClientGUI();

  void			appendLog(const QString &);
  void			addContact(const QString &);
  QString		getSelected() const;
  void			deleteSelected();
  void			deleteNamed(const QString&);
  const QString		getInitialContactName() const;
  QString		getNewContactName() const;
  QString		getNewContactKey() const;
  QString		getContactIp() const;

  QString		getRootName() const;
  QString		getRootKey() const;
  QString		getRootIP() const;
  QString		getRootPort() const;

  void			createAddContact(const QString&, const QString&, const QString&);
  void			createRootNodeGui(const QString &, const QString &, const QString &, quint16);

  void			createConfigGui();
  const QMap<QString, QVariant>* getChangesInConfig() const;

  void			printError(const QString &, const int time = 5);
  void			connected();
  void			disconnected();
private:
  AddContactGui*	_addContact;
  RootNodeGui*		_rootNode;
  ConfigGui*		_config;
  Controller		_controller;
  QTimer		_timer;
  bool			_isConnected;
signals:

public slots:
    void		createAddContact();
    void		deleteAddContact();
    void		addContactDeleted();

    void		rootNodeGuiDeleted();
    void		deleteRootNode();

    void		configGuiDeleted();

    void		showLogWidget();

    void		shutdown();
    void		restart();
    void		start();

    void		deleteConfig();

    void		about();
      void		aboutQt();
};

#endif // CLIENTGUI_H
