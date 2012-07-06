#include <QDebug>
#include "modellog.h"
#include "controller.h"

const QString ModelLog::_name = "ConnectedPeer";

ModelLog::ModelLog(Controller &c)
  :_controller(c)
{

}


const QString &ModelLog::getObjectName() const
 {
   return _name;
 }


const QMap<QString, QString> &ModelLog::getData() const
{
  qDebug() << "asking for void data in model log";
  return *new QMap<QString, QString>;
}

void  ModelLog::feedData(const QString &commande, const QMap<QString, QVariant> &data)
{
  qDebug() << commande;
  if (commande == "Add")
    {
      QString line = "New client at address :";
      line += data["IP"].toString();
      _controller.appendLog(line);
      qDebug() << line;
    }
}
