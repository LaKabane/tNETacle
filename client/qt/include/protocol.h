#ifndef _PROTOCOL_H
# define _PROTOCOL_H

#include <QString>
#include <QVector>

class Protocol
{
public :
  static QString add(const QString&, const QVector<QString>&);
  static QString delet(const QString&, const QVector<QString>&);

private:
  Protocol();
  ~Protocol();
};

#endif // !_PROTOCOL_H
