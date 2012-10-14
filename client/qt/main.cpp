#include <vector>
#include <iostream>

#include <QApplication>
#include "tclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TClient client;
    client.show();


    return app.exec();
}
