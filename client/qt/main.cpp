#include <vector>
#include <iostream>

#include <QApplication>
#include "qclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QClient client;
    client.show();


    return app.exec();
}
