#ifndef ADDCONTACTGUI_H
#define ADDCONTACTGUI_H

#include <QString>
#include <QWidget>
#include "ui_addContact.h"
#include "contact.h"
#include "controller.h"

class ClientGUI;

namespace UI {
    class addContactGui;
}

class addContactGui : public QWidget, private Ui::addContactGui
{
  Q_OBJECT
  public:
  QString getNewContactName() const;
  QString getNewContactKey() const;
  explicit addContactGui(Controller &client, QWidget* parents = 0);
    ~addContactGui();
  QDialogButtonBox* getOkOrReject() const {return okOrReject;}

private:
  Controller  &_controller;

private slots:
  void sendContact();
};

#endif // ADDCONTACTGUI_H
