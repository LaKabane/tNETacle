#ifndef MODELROOTNODE_H_
# define MODELROOTNODE_H_

#include <QMap>
#include <QString>
#include "imodel.h"

class Controller;

class ModelRootNode : public IModel
{
 public:
  ModelRootNode(Controller &);
  virtual		~ModelRootNode() {}

  virtual void		feedData(const QString&, const QVariant&);

  const QString		getKey() const;
  const QString		getName() const;
  const QString		getIP() const;
  const QString		getPort() const;
  void			changeRootNode(const QString& name, const QString& pubkey,
				       const QString& ip, const QString& port);
private:
  Controller&			_controller;
  static const QString		_name;
  QMap<QString, QVariant>	_details;

protected: // from IModel
  virtual const QString&		getObjectName() const;
  virtual const QMap<QString, QVariant>*	getData() const;
};

#endif /* !MODELROOTNODE_H */
