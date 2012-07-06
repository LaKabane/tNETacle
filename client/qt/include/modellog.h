#ifndef MODEL_LOG_H_
# define MODEL_LOG_H_

#include "model.h"

class Controller;

class ModelLog : public Model
{
public:
  ModelLog(Controller &);
  virtual const QString &getObjectName() const;
  virtual const QMap<QString, QString> &getData() const;
  virtual void feedData(const QString &, const QMap<QString, QVariant> &);
  static const QString _name;
  Controller    &_controller;
};

#endif /* !MODEL_LOG_H_ */
