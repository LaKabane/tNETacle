#include <QDebug>
#include "configgui.h"
#include <QWidget>
#include "clientgui.h"

ConfigGui::ConfigGui(Controller &controller, ClientGUI &win)
  : QWidget(0), _controller(controller), _view(win)
{
  this->setAttribute(Qt::WA_DeleteOnClose);

  setupUi(this);
}

ConfigGui::~ConfigGui()
{
}
