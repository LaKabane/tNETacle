#ifndef ICLIENTGUI_H
#define ICLIENTGUI_H

#include <QString>

class IClientGUI
{
public :
    virtual QWidget*    getHeader() const = 0;
    virtual QWidget*    getBody() const = 0;
};

#endif // ICLIENTGUI_H
