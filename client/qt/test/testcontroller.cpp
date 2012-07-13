#include "testcontroller.h"
#include "controller.h"
#include "stubclientgui.h"

void TestController::testRootNode_data()
{
  QTest::addColumn<QString>("name");
  QTest::addColumn<QString>("ip");
  QTest::addColumn<QString>("port");
  QTest::addColumn<QString>("key");
  QTest::addColumn<bool>("result");

  QTest::newRow("valid") << "Valid_name" << "192.168.1.1" << "4242" << "154651355151151" << true;
  QTest::newRow("valid2") << "Valid" << "10.0.0.1" << "4242" << "154651355151151" << true;

  QTest::newRow("no public key") << "Valid name" << "192.168.1.1" << "4242" << "" << false;


  QTest::newRow("no name") << "" << "192.168.1.1" << "4242" << "16515165" << false;
  QTest::newRow("bad name, dash") << "test-test" << "192.168.1.1" << "4242" << "16515165" << false;
  QTest::newRow("bad name, star") << "test*test" << "192.168.1.1" << "4242" << "16515165" << false;


  QTest::newRow("no IP adress")     << "Valid Name" << "" << "4242" << "1315151515" << false;
  QTest::newRow("bad ip, one number") << "test" << "192" << "4242" << "16515165" << false;
  QTest::newRow("bad ip, two numbers") << "test" << "192.168" << "4242" << "16515165" << false;
  QTest::newRow("bad ip, three numbers") << "test" << "192.168.1" << "4242" << "16515165" << false;
  QTest::newRow("bad ip, five numbers") << "test" << "192.168.1.1.1" << "4242" << "16515165" << false;
  QTest::newRow("bad ip, too much dot") << "test" << "192..168.1.1" << "4242" << "16515165" << false;
  QTest::newRow("bad ip, letter1") << "test" << "a.168.1.1" << "4242" << "16515165" << false;
  QTest::newRow("bad ip, letter2") << "test" << "192.a.1.1" << "4242" << "16515165" << false;
  QTest::newRow("bad ip, letter3") << "test" << "192.168.1a.1" << "4242" << "16515165" << false;
  QTest::newRow("bad ip, letter4") << "test" << "192.168.1.a" << "4242" << "16515165" << false;


  QTest::newRow("no port")     << "Valid Name" << "192.168.1.1" << "" << "1315151515" << false;
  QTest::newRow("bad port, letters") << "test" << "192.168.1.1" << "aa" << "16515165" << false;
  QTest::newRow("bad port, numer + letters") << "test" << "192.168.1.1" << "45aa" << "16515165" << false;
  QTest::newRow("bad port, too big") << "test" << "192.168.1.1" << "454242" << "16515165" << false;
}

void TestController::testRootNode()
{
  QFETCH(QString, name);
  QFETCH(QString, ip);
  QFETCH(QString, port);
  QFETCH(QString, key);
  QFETCH(bool, result);

  IClientGUI* stubClient = new StubClientGUI;
  dynamic_cast<StubClientGUI*>(stubClient)->_rootName = name;
  dynamic_cast<StubClientGUI*>(stubClient)->_rootip = ip;
  dynamic_cast<StubClientGUI*>(stubClient)->_rootPort = port;
  dynamic_cast<StubClientGUI*>(stubClient)->_rootPubkey = key;
  Controller c(stubClient);
  QCOMPARE(c.changeRootNode(), result);
  delete stubClient;
}

void TestController::testAddContact_data()
{
  QTest::addColumn<QString>("name");
  QTest::addColumn<QString>("key");
  QTest::addColumn<bool>("result");

  QTest::newRow("valid") << "Valid_name" << "154651355151151" << true;
  QTest::newRow("valid2") << "Valid_name" << "154651355151151" << true;
  QTest::newRow("no public key") << "Valid name" << "" << false;

  QTest::newRow("no name") << "" << "16515165" << false;
  QTest::newRow("bad name, dash") << "test-test" << "16515165" << false;
  QTest::newRow("bad name, star") << "test*test" << "16515165" << false;
}

void TestController::testAddContact()
{
  QFETCH(QString, name);
  QFETCH(QString, key);
  QFETCH(bool, result);

  IClientGUI* stubClient = new StubClientGUI;
  dynamic_cast<StubClientGUI*>(stubClient)->_contactName = name;
  dynamic_cast<StubClientGUI*>(stubClient)->_contactPubkey = key;
  Controller c(stubClient);
  QCOMPARE(c.addContact(), result);
  delete stubClient;
}


#include "moc_controller.cpp"
#include "moc_network.cpp"
#include "moc_clientgui.cpp"
#include "moc_configgui.cpp"
#include "moc_addcontactgui.cpp"
#include "moc_rootnodegui.cpp"
