#ifndef CONFIGGUI_H
#define CONFIGGUI_H

#include <QString>
#include <QWidget>
#include "iclientgui.h"
#include "ui_config.h"
#include "controller.h"

namespace Ui {
    class Config;
}

class IClientGUI;

class ConfigGui : public QWidget, public Ui::Config
{
  Q_OBJECT
  public:
  ConfigGui(Controller&, IClientGUI*);
  virtual ~ConfigGui();

  const QMap<QString, QVariant>* getChanges() const;

private:
  Controller&			_controller;
  IClientGUI*			_view;
  QMap<QString, QVariant>	_changes;

  void				displayConfigMenu();
  QWidget*			createMenu(const QMap<QString, QVariant>&);
};

#endif // CONFIGGUI_H
