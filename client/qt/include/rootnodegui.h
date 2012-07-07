#ifndef ROOTNODEGUI_H
#define ROOTNODEGUI_H

#include <QString>
#include <QWidget>
#include "clientgui.h"
#include "ui_rootnode.h"
#include "contact.h"
#include "controller.h"

class ClientGUI;

namespace UI {
    class RootNodeGui;
}

class RootNodeGui : public QWidget, private Ui::RootNodeGui
{
  Q_OBJECT
  public:
  const QString getInitialContactName() const;
  const QString getRootName() const;
  const QString getRootKey() const;
  const QString getRootIP() const;
  const QString getRootPort() const;
  explicit RootNodeGui(Controller &controller, ClientGUI &view, const QString &name, const QString &, const QString &, const quint16 port);
  explicit RootNodeGui(Controller &controller, ClientGUI &view);
  virtual ~RootNodeGui();
  QDialogButtonBox* getOkOrReject() const {return okOrReject;}

private:
  Controller  &_controller;
  ClientGUI &_view;
  const QString _initialName;
private slots:
  void sendRootNode();
};

#endif // ROOTNODEGUI_H
