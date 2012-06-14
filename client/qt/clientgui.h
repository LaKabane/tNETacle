#ifndef CLIENTGUI_H
#define CLIENTGUI_H

#include <QMainWindow>
#include "ui_client.h"
#include "Controller.h"
#include "contact.h"

namespace Ui {
    class ClientGUI;
}

class addContactGui;
class rootNodeGui;

class ClientGUI : public QMainWindow, private Ui::ClientGUI
{
    Q_OBJECT
public:
    explicit ClientGUI(QMainWindow *parent = 0);
    ~ClientGUI();
  //  void addContact(Contact* c);
  void addContact(const QString &);
  QString getSelected() const;
  void    deleteSelected();
  const QString getInitialContactName() const;
  QString getNewContactName() const;
  QString getNewContactKey() const;

  QString getRootName() const;
  QString getRootKey() const;
  QString getRootIP() const;

  void    createAddContact(const QString &, const QString &);
  void    createRootNodeGui(const QString &, const QString &, const QString &);

private:
    addContactGui*  _addContact;
	rootNodeGui*	_rootNode;
    Controller      _controller;

signals:

public slots:
    void            createAddContact();
    void            deleteAddContact();
    void  addContactDeleted();
	void  rootNodeGuiDeleted();
	void            deleteRootNode();

};

#endif // CLIENTGUI_H
