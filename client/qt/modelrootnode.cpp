#include <QDebug>
#include "modelrootnode.h"
#include "controller.h"

const QString ModelRootNode::_name = "RootNode";

ModelRootNode::ModelRootNode(Controller& c)
  : _controller(c), _details()
{
}

const QString&	ModelRootNode::getObjectName() const
 {
   return _name;
 }


const QMap<QString, QString>* ModelRootNode::getData() const
{
  return &_details;
}

void		ModelRootNode::feedData(const QString&, const QMap<QString, QVariant>&)
{
}

const QString	ModelRootNode::getKey() const
{
  if (_details.contains("pubkey") == false)
    return "";
  return this->_details["pubkey"];
}

const QString	ModelRootNode::getName() const
{
  if (_details.contains("name") == false)
    return "";
  return this->_details["name"];
}

const QString	ModelRootNode::getIP() const
{
  if (_details.contains("ip") == false)
    return "";
  return this->_details["ip"];
}

const QString	ModelRootNode::getPort() const
{
  if (_details.contains("port") == false)
    return "";
  return this->_details["port"];
}

void		ModelRootNode::changeRootNode(const QString& name, const QString& pubkey, const QString& ip, const QString& port)
{
  _details["name"] = name;
  _details["pubkey"] = pubkey;
  _details["ip"] = ip;
  _details["port"] = port;
}
