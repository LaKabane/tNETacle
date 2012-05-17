#ifndef CLIENTGUI_H
#define CLIENTGUI_H

#include <QMainWindow>
#include "ui_client.h"
#include "controler.h"

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

    void addContact(Contact* c);

private:
    addContactGui*  _addContact;
    Controler*      _controler;

signals:

public slots:
    void            createAddContact();
    void            deleteAddContact();

};

#endif // CLIENTGUI_H
