#ifndef ADDCONTACTGUI_H
#define ADDCONTACTGUI_H

#include <QString>
#include <QWidget>
#include "clientgui.h"
#include "ui_addcontact.h"
#include "controller.h"

namespace Ui {
    class AddContactGui;
}

class ClientGUI;

class AddContactGui : public QWidget, public Ui::AddContactGui
{
  Q_OBJECT
  public:
  QString getNewContactName() const;
  const QString getInitialContactName() const;
  QString getNewContactKey() const;
  explicit AddContactGui(Controller &controller, ClientGUI &view, const QString &name, const QString &key);

  explicit AddContactGui(Controller &controller, ClientGUI &view);
  virtual ~AddContactGui();
  QDialogButtonBox* getOkOrReject() const {return okOrReject;}

public slots:
  void	openPubKey();

private:
  Controller  &_controller;
  ClientGUI &_view;
  const QString _initialName;

private slots:
  void sendContact();
};

#endif // ADDCONTACTGUI_H
