#ifndef CONFIGGUI_H
#define CONFIGGUI_H

#include <QString>
#include <QWidget>
#include "clientgui.h"
#include "ui_config.h"
#include "controller.h"

namespace Ui {
    class Config;
}

class ClientGUI;

class ConfigGui : public QWidget, public Ui::Config
{
  Q_OBJECT
  public:
  ConfigGui(Controller&, ClientGUI&);
  ~ConfigGui();

  private:
  Controller&	_controller;
  ClientGUI&	_view;
};

#endif // CONFIGGUI_H
