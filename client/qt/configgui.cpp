#include <QDebug>
#include "configgui.h"
#include <QWidget>
#include "clientgui.h"

ConfigGui::ConfigGui(Controller &controller, IClientGUI* win)
  : QWidget(0), _controller(controller), _view(win), _changes()
{
  this->setAttribute(Qt::WA_DeleteOnClose);

  setupUi(this);
  QObject::connect(okOrReject, SIGNAL(accepted()), &_controller, SLOT(changeConfig()));
  QObject::connect(okOrReject, SIGNAL(rejected()), dynamic_cast<ClientGUI*>(_view), SLOT(deleteConfig()));
}

ConfigGui::~ConfigGui()
{
}

const QMap<QString, QVariant>* ConfigGui::getChanges() const
{
  if (_changes.size() == 0)
    return 0;
  return &_changes;
}
