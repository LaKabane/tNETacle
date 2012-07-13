#include "testcontact.h"
#include "modelcontact.h"
#include "controller.h"
#include "exception.h"
#include "stubclientgui.h"
#include <QVector>

void TestContact::testAddContact_data()
{
  QTest::addColumn<QString>("name");
  QTest::addColumn<QString>("key");
  QTest::addColumn<bool>("result");

  QTest::newRow("valid") << "Valid_name" << "154651355151151" << true;
}

void TestContact::testAddContact()
{
  QFETCH(QString, name);
  QFETCH(QString, key);
  QFETCH(bool, result);

  bool res;
  IClientGUI* stubClient = new StubClientGUI;
  Controller c(stubClient);
  ModelContact cont(c, stubClient);
  QVector<QString> v;
  v.append(name);
  v.append(key);
  try
    {
      cont.addContact(v);
      res = true;
    }
  catch (Exception* e)
    {
      res = false;
    }
  QCOMPARE(res, result);
  delete stubClient;
}
