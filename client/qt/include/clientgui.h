#ifndef CLIENTGUI_H
#define CLIENTGUI_H

#include <QMainWindow>
#include <QTimer>
#include "ui_client.h"
#include "controller.h"
#include "contact.h"

namespace Ui {
    class ClientGUI;
}

class addContactGui;
class rootNodeGui;

class ClientGUI : public QMainWindow, public Ui::ClientGUI
{
    Q_OBJECT
public:
    explicit ClientGUI(QMainWindow *parent = 0);
    ~ClientGUI();
  //  void addContact(Contact* c);
  void appendLog(const QString &);
  void addContact(const QString &);
  QString getSelected() const;
  void    deleteSelected();
  const QString getInitialContactName() const;
  QString getNewContactName() const;
  QString getNewContactKey() const;

  QString getRootName() const;
  QString getRootKey() const;
  QString getRootIP() const;
  QString getRootPort() const;

  void    createAddContact(const QString &, const QString &);
  void    createRootNodeGui(const QString &, const QString &, const QString &, quint16 );
  void    printError(const QString &);
private:
  addContactGui*  _addContact;
  rootNodeGui*	_rootNode;
  Controller      _controller;
  QTimer        _timer;
signals:

public slots:
    void            createAddContact();
    void            deleteAddContact();
    void  addContactDeleted();
	void  rootNodeGuiDeleted();
	void            deleteRootNode();

};

#endif // CLIENTGUI_H
