#include <QMap>
#include <QVariant>
#include "imodel.h"
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


const QByteArray IModel::toJson() const
{
  // QJson::Serializer serializer;
  // QVariant name = QMap<QString, QVariant = this->getObjectName;
  // QVariant obj(this->getObjectName, Model::mapToVar(this->getData()));
  // return serializer.serialize(obj);

  QByteArray json;
  const QMap<QString, QVariant>* datas = this->getData();
  json += "{\n\"";
  json += this->getObjectName();
  json += "\":[";
  if (datas != 0)
    {
      QMap<QString, QVariant>::const_iterator it(datas->begin());
      for (const QMap<QString, QVariant>::const_iterator it_end = datas->end();
	   it != it_end; ++it)
	{
	  json += "{\nname:\"";
	  json += it.key();
	  json += "\"\nkey:\"";
	  json += it.value().toString();
	  json += "\"\n}\n,";
	}
    }
  json[json.size() - 1] = ' ';
  json += "]\n}";
  return json;
}
