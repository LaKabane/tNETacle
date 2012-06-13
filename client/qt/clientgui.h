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
  QString getNewContactName() const;
  const QString getInitialContactName() const;
  QString getNewContactKey() const;
  void    createAddContact(const QString &, const QString &);
private:
    addContactGui*  _addContact;
    Controller      _controller;

signals:

public slots:
    void            createAddContact();
    void            deleteAddContact();
    void  addContactDeleted();

};

#endif // CLIENTGUI_H
