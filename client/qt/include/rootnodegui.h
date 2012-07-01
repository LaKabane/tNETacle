#ifndef ROOTNODEGUI_H
#define ROOTNODEGUI_H

#include <QString>
#include <QWidget>
#include "clientgui.h"
#include "ui_rootNode.h"
#include "contact.h"
#include "Controller.h"

class ClientGUI;

namespace UI {
    class rootNodeGui;
}

class rootNodeGui : public QWidget, private Ui::rootNodeGui
{
  Q_OBJECT
  public:
  const QString getInitialContactName() const;
  QString getRootName() const;
  QString getRootKey() const;
  QString getRootIP() const;
  explicit rootNodeGui(Controller &controller, ClientGUI &view, const QString &name, const QString &, const QString &);

  explicit rootNodeGui(Controller &controller, ClientGUI &view);
  virtual ~rootNodeGui();
  QDialogButtonBox* getOkOrReject() const {return okOrReject;}

private:
  Controller  &_controller;
  ClientGUI &_view;
  const QString _initialName;
private slots:
  void sendRootNode();
};

#endif // ROOTNODEGUI_H
