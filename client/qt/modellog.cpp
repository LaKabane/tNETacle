#include <QDebug>
#include "modellog.h"
#include "controller.h"
#include "exception.h"

const QString ModelLog::_name = "Log";

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

void  ModelLog::feedData(const QString &command, const QVariant& data)
{
  if (command == "AddLog")
    {
      QMap<QString, QVariant> logs = data.toMap();
      QString log("");
      if (logs.contains("Log") == true)
	log += logs["Log"].toString();
      if (log != "")
      addLog(log);
    }
  else
    throw new Exception("Error: command does not exist!");
}

void	ModelLog::addLog(const QString& log)
{
  _controller.appendLog(log);
}
