#include "tclient.h"

TClient::TClient(QMainWindow* parent, Controller* controller) :
  QMainWindow(parent),
  _controller(controller),
  _header(0),
  _body(0)
{
    // _header = new header(this);
    // _body = new bodyconnexion(this);
    setupUi(this);
}
