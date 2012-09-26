#include <QDebug>
#include "protocol.h"

QString	Protocol::add(const QString& object, const QVector<QString>& v)
{
  QString	json("");

  if (v.size() < 3 || object == "")
    return json;

  json = "{";
  json += "\"Add";
  json += object;
  json += "\": {";

  // TODO : make it generic, work only for Contact
  json += "\"Name\":\"";
  json += v[0];
  json += "\",";
  json += "\"Key\": \"";
  json += v[1];
  json += "\",";
  json += "\"Ip\": \"";
  json += v[2];
  json += "\"";
  //!TODO

  json += "}";
  json += "}\n";
  return json;
}

QString	Protocol::delet(const QString& object, const QVector<QString>& v)
{
  QString	json("");

  if (v.size() < 1 || object == "")
    return json;

  json = "{";
  json += "\"Delete";
  json += object;
  json += "\": {";

  // TODO : make it generic, work only for Contact
  json += "\"Name\": \"";
  json += v[0];
  json += "\"";
  //!TODO

  json += "}";
  json += "}\n";

  return json;
}
