
#ifndef EXCEPTION_H_
# define EXCEPTION_H_

#include <QString>

class Exception
{
public:
  Exception(QString c) :
    _message(c) { };
  const QString &getMessage() const
  { return _message;}
private:
  const QString _message;
};

#endif /* !EXCEPTION_H_ */
