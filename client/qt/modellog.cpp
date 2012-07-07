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


const QMap<QString, QVariant>* ModelLog::getData() const
{
  qDebug() << "asking for void data in model log";
  return 0;
}

void  ModelLog::feedData(const QString &commande, const QMap<QString, QVariant> &data)
{
  qDebug() << commande;
}
