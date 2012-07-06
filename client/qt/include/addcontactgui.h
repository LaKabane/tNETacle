#ifndef ADDCONTACTGUI_H
#define ADDCONTACTGUI_H

#include <QString>
#include <QWidget>
#include "clientgui.h"
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
  const QString getInitialContactName() const;
  QString getNewContactKey() const;
  explicit addContactGui(Controller &controller, ClientGUI &view, const QString &name, const QString &key);

  explicit addContactGui(Controller &controller, ClientGUI &view);
  virtual ~addContactGui();
  QDialogButtonBox* getOkOrReject() const {return okOrReject;}

private:
  Controller  &_controller;
  ClientGUI &_view;
  const QString _initialName;
private slots:
  void sendContact();
};

#endif // ADDCONTACTGUI_H
