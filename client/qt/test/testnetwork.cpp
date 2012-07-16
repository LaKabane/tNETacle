#include "testnetwork.h"
#include <QVector>
#include "controller.h"
#include "protocol.h"
#include "stubclientgui.h"

void TestNetwork::testIsConnected()
{
  IClientGUI* stub = new StubClientGUI;
  Controller c(stub);
  Network n(c);
  QCOMPARE(n.isConnected(), false);
}
