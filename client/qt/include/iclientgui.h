#ifndef ICLIENTGUI_H
#define ICLIENTGUI_H

#include <QString>

class IClientGUI
{
  //TODO : sort the function

public :
  virtual void		appendLog(const QString &) = 0;
  virtual void		addContact(const QString &) = 0;
  virtual QString	getSelected() const = 0;
  virtual void		deleteSelected() = 0;
  virtual const QString getInitialContactName() const = 0;
  virtual QString	getNewContactName() const = 0;
  virtual QString	getNewContactKey() const = 0;

  virtual QString	getRootName() const = 0;
  virtual QString	getRootKey() const = 0;
  virtual QString	getRootIP() const = 0;
  virtual QString	getRootPort() const = 0;

  virtual void		createAddContact(const QString &, const QString &) = 0;
  virtual void		createRootNodeGui(const QString &, const QString &, const QString &, quint16) = 0;
  virtual void		printError(const QString &) = 0;
  virtual void		createConfigGui() = 0;
  virtual void		connected() = 0;
  virtual void		disconnected() = 0;

public slots:
  virtual void		createAddContact() = 0;
  virtual void		deleteAddContact() = 0;
  virtual void		addContactDeleted() = 0;
  virtual void		rootNodeGuiDeleted() = 0;
  virtual void		deleteRootNode() = 0;
  virtual void		configGuiDeleted() = 0;
  virtual void		deleteConfig() = 0;
};

#endif // CLIENTGUI_H
