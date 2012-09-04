#include <QDebug>
#include <QMessageBox>
#include "clientgui.h"
#include <iostream>
#include "addcontactgui.h"
#include "rootnodegui.h"
#include "configgui.h"

ClientGUI::ClientGUI(QMainWindow *parent) :
    QMainWindow(parent),
    _addContact(0),
    _rootNode(0),
    _config(0),
    _controller(this),
    _timer(),
    _isConnected(false)
{
   setupUi(this);
   QObject::connect(actionAddContact, SIGNAL(activated()), this, SLOT(createAddContact()));
   QObject::connect(actionDeleteContact, SIGNAL(activated()), &_controller, SLOT(deleteContact()));
   QObject::connect(actionAddAContact, SIGNAL(activated()), this, SLOT(createAddContact()));
   QObject::connect(actionDeleteAContact, SIGNAL(activated()), &_controller, SLOT(deleteContact()));
   QObject::connect(ContactsList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), &_controller, SLOT(editContact(QListWidgetItem *)));

   QObject::connect(actionChangeRoot, SIGNAL(activated()), &_controller, SLOT(editRootNode()));
   QObject::connect(actionCore, SIGNAL(activated()), &_controller, SLOT(editRootNode()));

   QObject::connect(actionLog, SIGNAL(activated()), this, SLOT(showLogWidget()));

   QObject::connect(actionConfiguration, SIGNAL(activated()), &_controller, SLOT(editConfig()));
   QObject::connect(actionConfig, SIGNAL(activated()), &_controller, SLOT(editConfig()));

   QObject::connect(actionShutdown, SIGNAL(activated()), this, SLOT(shutdown()));
   QObject::connect(actionRestart, SIGNAL(activated()), this, SLOT(restart()));

   error->hide();
   log->hide();
   QObject::connect(&_timer, SIGNAL(timeout()), error, SLOT(hide()));

   QObject::connect(actionConnect, SIGNAL(activated()), this, SLOT(start()));
   QObject::connect(actionConnect2, SIGNAL(activated()), this, SLOT(start()));

   QObject::connect(actionAbout, SIGNAL(activated()), this, SLOT(about()));

   QObject::connect(actionAboutQt, SIGNAL(activated()), this, SLOT(aboutQt()));

   actionConnect->setVisible(true);
   actionConnect2->setVisible(true);
   actionAddContact->setVisible(false);
   actionDeleteContact->setVisible(false);
   actionAddAContact->setVisible(false);
   actionDeleteAContact->setVisible(false);
}

ClientGUI::~ClientGUI() {
}


void ClientGUI::appendLog(const QString &str)
{
  this->log->append(str);
}


void ClientGUI::printError(const QString &error, const int time)
{
  this->error->show();
  this->errorText->setText(error);
  this->_timer.start(time * 1000);
  qDebug() << error;
}

void    ClientGUI::deleteSelected()
{
  int pos = this->ContactsList->currentRow();
  delete this->ContactsList->currentItem();
  this->ContactsList->setCurrentRow(pos - 1);
}

void    ClientGUI::deleteNamed(const QString& name)
{
  int pos = -1;

  for (unsigned int i = 0; i < this->ContactsList->count(); ++i)
    {
      if (this->ContactsList->item(i)->text() == name)
	{
	  pos = i;
	  break;
	}
    }

  if (pos != -1)
    delete this->ContactsList->item(pos);
  this->ContactsList->setCurrentRow(pos - 1);
}

QString ClientGUI::getSelected() const
{
  if (this->ContactsList->currentItem() == false)
    return "";
  return this->ContactsList->currentItem()->text();
}

const QString ClientGUI::getInitialContactName() const
{
  if (!this->_addContact)
    return *new QString("");// TODO throw a fatal exception
  return this->_addContact->getInitialContactName();
}

QString ClientGUI::getNewContactName() const
{
  if (this->_addContact == false)
    return *new QString("");// TODO throw a fatal exception
  return this->_addContact->getNewContactName();
}

QString ClientGUI::getNewContactKey() const
{
  if (!this->_addContact)
    return *new QString("");// TODO throw a fatal exception
  return this->_addContact->getNewContactKey();
}

QString ClientGUI::getRootName() const
{
  if (!this->_rootNode)
    return *new QString("");// TODO throw a fatal exception
  return this->_rootNode->getRootName();
}

QString ClientGUI::getRootKey() const
{
  if (!this->_rootNode)
    return *new QString("");// TODO throw a fatal exception
  return this->_rootNode->getRootKey();
}

QString ClientGUI::getRootIP() const
{
  if (!this->_rootNode)
    return *new QString("");// TODO throw a fatal exception
  return this->_rootNode->getRootIP();
}


QString ClientGUI::getRootPort() const
{
  if (this->_rootNode == 0)
    return *new QString("");// TODO throw a fatal exception
  return this->_rootNode->getRootPort();
}

const QMap<QString, QVariant>* ClientGUI::getChangesInConfig() const
{
  if (this->_config == 0)
    return 0;
  return this->_config->getChanges();
}


void ClientGUI::createAddContact(const QString& name, const QString &key) {
  if (_addContact)
    return ;
  _addContact = new AddContactGui(this->_controller, *this, name, key);

  QObject::connect(_addContact, SIGNAL(destroyed()), this, SLOT(addContactDeleted()));
  _addContact->show();
}



void ClientGUI::createAddContact() {
  if (_addContact)
    return ;
  _addContact = new AddContactGui(this->_controller, *this);

  QObject::connect(_addContact, SIGNAL(destroyed()), this, SLOT(addContactDeleted()));
  _addContact->show();
}

void ClientGUI::deleteAddContact() {
  _addContact->close();
}

void ClientGUI::deleteRootNode() {
  _rootNode->close();
}

void ClientGUI::deleteConfig() {
  _config->close();
}

void ClientGUI::addContactDeleted() {
    _addContact = 0;
}

void ClientGUI::addContact(const QString &str)
{
  this->ContactsList->addItem(str);
}

void ClientGUI::createRootNodeGui(const QString& name, const QString &key, const QString &IP, quint16 port) {
  if (_rootNode)
    return ;
  _rootNode = new RootNodeGui(this->_controller, *this, name, key, IP, port);

    QObject::connect(_rootNode, SIGNAL(destroyed()), this, SLOT(rootNodeGuiDeleted()));
  _rootNode->show();
}

void ClientGUI::createConfigGui() {
  if (_config)
    return ;
  _config = new ConfigGui(this->_controller, this);

    QObject::connect(_config, SIGNAL(destroyed()), this, SLOT(configGuiDeleted()));
  _config->show();
}

void ClientGUI::rootNodeGuiDeleted() {
    _rootNode = 0;
}

void ClientGUI::configGuiDeleted() {
    _config = 0;
}

void ClientGUI::showLogWidget() {
  if (log->isVisible() == true)
    log->hide();
  else
    log->show();
}

void ClientGUI::shutdown()
{
  QMessageBox msgBox(QMessageBox::Warning, "Shutdown", "Shutdown!");
  msgBox.setInformativeText("Do you want to shutdown?");
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Ok);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Ok)
    _controller.shutdown();
}

void ClientGUI::restart()
{
  QMessageBox msgBox(QMessageBox::Warning, "Restart", "Restart!");
  msgBox.setInformativeText("Do you want to restart the core connection?");
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Ok);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Ok)
    _controller.restart();
}

void ClientGUI::start()
{
  _controller.restart();
}

void ClientGUI::connected()
{
  actionConnect->setVisible(false);
  actionConnect2->setVisible(false);
  QIcon ico;
  ico.addFile(QString::fromUtf8(":/tools/root_connected.png"), QSize(), QIcon::Normal, QIcon::Off);
  actionChangeRoot->setIcon(ico);
  actionCore->setIcon(ico);
  _isConnected = true;
  actionAddContact->setVisible(true);
  actionDeleteContact->setVisible(true);
  actionAddAContact->setVisible(true);
  actionDeleteAContact->setVisible(true);
}

void ClientGUI::disconnected()
{
  actionConnect->setVisible(true);
  actionConnect2->setVisible(true);
  this->ContactsList->clear();
  QIcon ico;
  ico.addFile(QString::fromUtf8(":/tools/root_notconnected.png"), QSize(), QIcon::Normal, QIcon::Off);
  actionChangeRoot->setIcon(ico);
  actionCore->setIcon(ico);
  _isConnected = false;
  actionAddContact->setVisible(false);
  actionDeleteContact->setVisible(false);
  actionAddAContact->setVisible(false);
  actionDeleteAContact->setVisible(false);
}

void ClientGUI::about()
{
   QMessageBox messageAbout;
   QString message = tr(QString::fromUtf8("tNETacle permet de créer de façon simple des réseaux privés virtuels hautement configurables. Grâce à son interface claire il met à la portée de tous l'utilisation de ces réseaux et est intrinsèquement résistant aux pannes et respectueux de la vie privée du fait de son architecture décentralisée et de son chiffrement fort.").toStdString().c_str());
   messageAbout.about(0, "tNETacle", message);
}

void ClientGUI::aboutQt()
{
   QMessageBox messageAbout;
   messageAbout.aboutQt(0, "tNETacle");
}
