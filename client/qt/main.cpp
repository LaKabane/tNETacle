#include "contact.h"
#include "contactsList.h"
#include "controler.h"
#include <vector>
#include <iostream>

#include <QApplication>
#include "clientgui.h"
#include "addcontactgui.h"

int main(int argc, char *argv[])
{
  //Controler	ct;
  //return !ct.test();
    QApplication app(argc, argv);
    ClientGUI client;
    client.show();


    return app.exec();
}
