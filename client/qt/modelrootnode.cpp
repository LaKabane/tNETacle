/*
 * Copyright (c) 2012 Florent Tribouilloy <tribou_f AT epitech DOT net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <QDebug>
#include "modelrootnode.h"
#include "controller.h"

const QString ModelRootNode::_name = "RootNode";

ModelRootNode::ModelRootNode(Controller& c)
  : _controller(c), _details()
{
  _details["name"] = "test";
  _details["pubkey"] = "pubkey";
  _details["ip"] = "127.0.0.1";
  _details["port"] = "4243";
}

const QString&	ModelRootNode::getObjectName() const
{
  return _name;
}

const QMap<QString, QVariant>* ModelRootNode::getData() const
{
  return &_details;
}

void		ModelRootNode::feedData(const QString&, const QVariant&)
{
}

const QString	ModelRootNode::getKey() const
{
  if (_details.contains("pubkey") == false)
    return "";
  return this->_details["pubkey"].toString();
}

const QString	ModelRootNode::getName() const
{
  if (_details.contains("name") == false)
    return "";
  return this->_details["name"].toString();
}

const QString	ModelRootNode::getIP() const
{
  if (_details.contains("ip") == false)
    return "";
  return this->_details["ip"].toString();
}

const QString	ModelRootNode::getPort() const
{
  if (_details.contains("port") == false)
    return "";
  return this->_details["port"].toString();
}

void		ModelRootNode::changeRootNode(const QString& name, const QString& pubkey, const QString& ip, const QString& port)
{
  _details["name"] = name;
  _details["pubkey"] = pubkey;
  _details["ip"] = ip;
  _details["port"] = port;
}
