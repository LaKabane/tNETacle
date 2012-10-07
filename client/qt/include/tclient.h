#ifndef TCLIENT_H_
# define TCLIENT_H_

#include <QMainWindow>
#include "controller.h"
#include "ui_tclient.h"

namespace Ui {
    class TClient;
}

class TClient : public QMainWindow, Ui::TClient
{
    Q_OBJECT

public:
    explicit   TClient(QMainWindow* parent = 0, Controller* controller = 0);
    virtual    ~TClient() {};

private:
    Controller*    _controller;
    QWidget*    _header;
    QWidget*    _body;
};

#endif /* !TCLIENT_H_ */
