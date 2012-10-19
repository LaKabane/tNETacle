#ifndef ICLIENTGUI_H
#define ICLIENTGUI_H

#include <QString>

class IClientGUI
{
public :
    virtual QWidget*    getHeader() const = 0;
    virtual QWidget*    getBody() const = 0;
    virtual void        changeNextBody() = 0;
    virtual void        changePrevBody() = 0;
    virtual void        connected() = 0;
    virtual void        disconnected() = 0;
};

#endif // ICLIENTGUI_H
