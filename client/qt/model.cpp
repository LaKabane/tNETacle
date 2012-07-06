#include <QMap>
#include <QVariant>
#include "model.h"
#include <qjson/serializer.h>


// const QMap<QString, QVariant> Model::mapToVar(const QMap<QString, QString> &map)
// {
//   QMap<QString, QVariant> var;
//   QMap<QString, QString>::const_iterator it;
//   const QMap<QString, QString>::const_iterator it_end = map.end();
//   for (; it != it_end; ++it)
//     {
//       QVariant str = it.value();
//       var[it.key()] = str;
//     }
//   return var;
// }


const QByteArray Model::toJson() const
{
  // QJson::Serializer serializer;
  // QVariant name = QMap<QString, QVariant = this->getObjectName;
  // QVariant obj(this->getObjectName, Model::mapToVar(this->getData()));
  // return serializer.serialize(obj);

  QByteArray json;
  json += "{\n\"";
  json += this->getObjectName();
  json += "\":[";
  QMap<QString, QString>::const_iterator it(this->getData().begin());
  for (const QMap<QString, QString>::const_iterator it_end = this->getData().end();
       it != it_end; ++it)
    {
      json += "{\nname:\"";
      json += it.key();
      json += "\"\nkey:\"";
      json += it.value();
      json += "\"\n}\n,";
    }
  json[json.size() - 1] = ' ';
  json += "]\n}";
  return json;
}
