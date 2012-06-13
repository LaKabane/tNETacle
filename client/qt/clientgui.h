#ifndef CLIENTGUI_H
#define CLIENTGUI_H

#include <QMainWindow>
#include "ui_client.h"
#include "controller.h"
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
  QString getNewContactName() const;
  QString getNewContactKey() const;
private:
    addContactGui*  _addContact;
    Controller      _controller;

signals:

public slots:
    void            createAddContact();
    void            deleteAddContact();

};

#endif // CLIENTGUI_H
