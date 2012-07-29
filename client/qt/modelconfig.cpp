#include <QDebug>
#include "modelconfig.h"
#include "controller.h"

const QString ModelConfig::_name = "Configuration";

ModelConfig::ModelConfig(Controller &c)
  : _controller(c)
{
  //TODO : test for the gui, erase at the end
  _config = new QMap<QString, QVariant>();
  QMap<QString, QVariant> map63;
  map63["element163-1"] = "value";
  map63["element163-2"] = "value";

  QMap<QString, QVariant> map6;
  map6["element16-1"] = "value";
  map6["element16-2"] = "value";
  map6["element16-3"] = map63;

  QMap<QString, QVariant> map7;
  map7["element17-1"] = "value";
  map7["element17-2"] = "value";

  QMap<QString, QVariant> map;
  map["element1-1"] = "value";
  map["element1-2"] = "value";
  map["element1-3"] = "value";
  map["element1-4"] = "value";
  map["element1-5"] = "value";
  map["element1-6"] = map6;
  map["element1-7"] = map7;

  QMap<QString, QVariant> map2;
  map2["element2-1"] = "value";

  QMap<QString, QVariant> map3;
  map3["element3-1"] = "value";

  _config->insert("first menu", map);
  _config->insert("second menu", map2);
  _config->insert("third menu", map3);
}


const QString &ModelConfig::getObjectName() const
 {
   return _name;
 }


const QMap<QString, QVariant>* ModelConfig::getData() const
{
  return _config;
}

void  ModelConfig::feedData(const QString &commande, const QVariant&)
{
  qDebug() << commande;
}
