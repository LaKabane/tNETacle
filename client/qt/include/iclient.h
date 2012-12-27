#ifndef ICLIENT_H
#define ICLIENT_H

#include <QString>

class IClient
{
public :
    virtual QWidget*    getHeader() const = 0;
    virtual QWidget*    getBody() const = 0;
    virtual void        connected() = 0;
    virtual void        disconnected() = 0;
};

#endif // ICLIENT_H
