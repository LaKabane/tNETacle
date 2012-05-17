#ifndef ADDCONTACTGUI_H
#define ADDCONTACTGUI_H

#include <QWidget>
#include "ui_addContact.h"
#include "contact.h"

class ClientGUI;

namespace UI {
    class addContactGui;
}

class addContactGui : public QWidget, private Ui::addContactGui
{
    Q_OBJECT
public:
    explicit addContactGui(ClientGUI* client, QWidget* parents = 0);
    ~addContactGui();
    QDialogButtonBox* getOkOrReject() const {return okOrReject;}

private:
    ClientGUI*  _clientGui;

private slots:
        void sendContact();
};

#endif // ADDCONTACTGUI_H
