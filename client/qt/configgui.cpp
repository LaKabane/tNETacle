#include <QDebug>
#include "configgui.h"
#include <QWidget>
#include <QLineEdit>
#include <QFormLayout>
#include <QDockWidget>
#include "clientgui.h"

ConfigGui::ConfigGui(Controller &controller, IClientGUI* win)
  : QWidget(0), _controller(controller), _view(win), _changes()
{
  this->setAttribute(Qt::WA_DeleteOnClose);

  setupUi(this);
  QObject::connect(okOrReject, SIGNAL(accepted()), &_controller, SLOT(changeConfig()));
  QObject::connect(okOrReject, SIGNAL(rejected()), dynamic_cast<ClientGUI*>(_view), SLOT(deleteConfig()));
  this->displayConfigMenu();
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

void	ConfigGui::displayConfigMenu()
{
  const QMap<QString, QVariant>* menus = _controller.getConfigMenu();

  if (menus == 0)
    return ;

  QMap<QString, QVariant>::const_iterator it = menus->constBegin();
  QMap<QString, QVariant>::const_iterator ite = menus->constEnd();

  for (; it != ite; ++it)
    {
      if (it.value().canConvert(QVariant::Map) == true)
	{
	  QWidget* q = this->createMenu(it.value().toMap());
	  this->tabulation->addTab(q, it.key());
	}
    }
}

QWidget*	ConfigGui::createMenu(const QMap<QString, QVariant>& map)
{
  QMap<QString, QVariant>::const_iterator it = map.constBegin();
  QMap<QString, QVariant>::const_iterator ite = map.constEnd();
  if (map.size() == 0)
    return 0;
  QWidget* widget = new QWidget();
  QLayout* layout = new QVBoxLayout;

  for (; it != ite; ++it)
    {
      QWidget* interface = 0;
      if (it.value().canConvert(QVariant::Map) == true)
	{
	  interface = new QDockWidget(it.key());
	  dynamic_cast<QDockWidget*>(interface)->setFeatures(QDockWidget::DockWidgetMovable);

	  QWidget* dockContent = createMenu(it.value().toMap());

	  dynamic_cast<QDockWidget*>(interface)->setWidget(dockContent);
	}
      else if (it.value().canConvert(QVariant::String) == true)
	{
	  interface = new QWidget;
	  QFormLayout* formLayout = new QFormLayout;

	  QLabel* label = new QLabel();
	  label->setText(it.key());
	  QLineEdit* lineEdit = new QLineEdit();
	  lineEdit->setText(it.value().toString());

	  formLayout->addRow(label, lineEdit);
	  interface->setLayout(formLayout);
	}
      layout->addWidget(interface);
    }
  widget->setLayout(layout);
  return widget;
}
