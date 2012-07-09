#ifndef MODELCONFIG_H_
# define MODELCONFIG_H_

#include "imodel.h"

class Controller;

class ModelConfig : public IModel
{
public:
  ModelConfig(Controller &);
  virtual ~ModelConfig() {}

  virtual const QString &getObjectName() const;
  virtual const QMap<QString, QVariant>* getData() const;
  virtual void feedData(const QString &, const QMap<QString, QVariant> &);

private:
  static const QString _name;
  QMap<QString, QVariant>* _config;
  Controller    &_controller;
};

#endif /* !MODELCONFIG_H_ */
