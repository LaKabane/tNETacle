#include <vector>
#include <iostream>

#include <QApplication>
#include "clientgui.h"
#include "addcontactgui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ClientGUI client;
    client.show();


    return app.exec();
}
