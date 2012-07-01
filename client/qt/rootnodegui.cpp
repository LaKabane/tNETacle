#include <QDebug>
#include "rootnodegui.h"
#include <QWidget>
#include "contact.h"
#include "clientgui.h"

rootNodeGui::rootNodeGui(Controller &controller, ClientGUI &win)
  : QWidget(0), _controller(controller),_view(win), _initialName("")
{
  this->setAttribute(Qt::WA_DeleteOnClose);
  setupUi(this);
  QObject::connect(okOrReject, SIGNAL(accepted()), &_controller, SLOT(changeRootNode()));
  QObject::connect(okOrReject, SIGNAL(rejected()), &win, SLOT(deleteAddContact()));
}

rootNodeGui::rootNodeGui(Controller &controller, ClientGUI &win, const QString &name, const QString &key, const QString &IP, const quint16 port)
  : QWidget(0), _controller(controller),_view(win), _initialName(name)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);
    QObject::connect(okOrReject, SIGNAL(accepted()), &_controller, SLOT(changeRootNode()));
    //QObject::connect(okOrReject, SIGNAL(rejected()), &win, SLOT(deleteRootNode()));
    this->name->setText(name);
    this->pubKey->setText(key);
    this->IP->setText(IP);
    QString tmp;
    this->port->setText(tmp.setNum(port));
}


rootNodeGui::~rootNodeGui()
{

}

const QString rootNodeGui::getRootName() const
{
  return (this->name->text());
}

const QString rootNodeGui::getInitialContactName() const
{
  return this->_initialName;
}

const QString rootNodeGui::getRootKey() const
{
  return (this->pubKey->toPlainText());
}

const QString rootNodeGui::getRootIP() const
{
  return (this->IP->text());
}


const QString rootNodeGui::getRootPort() const
{
  return (this->port->text());
}

void rootNodeGui::sendRootNode() {
    // Contact* newContact = new Contact(this->nameLabel->text().toStdString(),
    //                                   this->keyText->toPlainText().toStdString());
    // std::cout << "Lololilolilol" << *newContact << std::endl;
    // _clientGui->rootNode(newContact);
}
