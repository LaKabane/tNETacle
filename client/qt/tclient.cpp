#include "tclient.h"
#include "theader.h"

TClient::TClient(QMainWindow* parent, Controller* controller) :
  QMainWindow(parent),
  _controller(controller),
  _header(0),
  _body(0)
{
    setupUi(this);
    _header = new THeader(this);
    // _body = new bodyconnexion(this);
}

TClient::~TClient()
{
}
