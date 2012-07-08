#include <QDebug>
#include "modelconfig.h"
#include "controller.h"

const QString ModelConfig::_name = "Configuration";

ModelConfig::ModelConfig(Controller &c)
  :_controller(c)
{
}


const QString &ModelConfig::getObjectName() const
 {
   return _name;
 }


const QMap<QString, QVariant>* ModelConfig::getData() const
{
  return _config;
}

void  ModelConfig::feedData(const QString &commande, const QMap<QString, QVariant>&)
{
  qDebug() << commande;
}
