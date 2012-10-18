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
