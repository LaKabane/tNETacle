#ifndef STUBCLIENTGUI_H
#define STUBCLIENTGUI_H

#include "iclientgui.h"

class StubClientGUI : public IClientGUI
{
public:
    explicit StubClientGUI() {}
    ~StubClientGUI();

	void addContact(const QString &) {}
	QString getSelected() const {return QString("");}
	void    deleteSelected() {}
	const QString getInitialContactName() const {return QString("");}

	QString getNewContactName() const {return _contactName;}
	QString getNewContactKey() const {return _contactPubkey;}

	QString getRootName() const {return _rootName;}
	QString getRootKey() const {return _rootPubkey;}
	QString getRootIP() const {return _rootip;}
	QString	getRootPort() const {return _rootPort;}

	void    createAddContact(const QString &, const QString &) {}
	void    createRootNodeGui(const QString &, const QString &, const QString &, quint16) {}

	QString	_rootName;
	QString	_rootip;
	QString	_rootPort;
	QString	_rootPubkey;

	QString	_contactName;
	QString	_contactPubkey;
	void printError(const QString&) {};
	void appendLog(const QString &) {};

	virtual void createConfigGui() {}
	virtual void connected() {}
	virtual void disconnected() {}
	virtual void configGuiDeleted() {}
	virtual void deleteConfig() {}

public slots:
	void	createAddContact() {}
	void	deleteAddContact() {}
	void	addContactDeleted() {}
	void	rootNodeGuiDeleted() {}
	void	deleteRootNode() {}

};

#endif // STUBCLIENTGUI_H
