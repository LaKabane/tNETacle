#include "testprotocol.h"
#include <QVector>
#include "protocol.h"

void TestProtocol::testAdd_data()
{
  QTest::addColumn<QString>("object");
  QTest::addColumn<QString>("name");
  QTest::addColumn<QString>("key");

  QTest::newRow("validAddContact") << "Contact" << "name" << "key";
  QTest::newRow("without object") << "" << "name" << "key";
  QTest::newRow("without name") << "Contact" << "" << "key";\
  QTest::newRow("without key") << "Contact" << "name" << "";
  QTest::newRow("without name && key") << "Contact" << "" << "";
}

void TestProtocol::testAdd()
{
  QFETCH(QString, object);
  QFETCH(QString, name);
  QFETCH(QString, key);

  QString result = "{\"Add" + object + "\": {\"" + name + "\": {\"Key\": \"" + key + "\"}}}\n";

  QVector<QString> v;
  v.append(name);
  v.append(key);


  if (object == "")
    {
      result = "";
    }

  if (name == "")
    {
      v.remove(0);
      result = "";
    }

  if (key == "")
    {
      v.remove(name == "" ? 0 : 1);
      result = "";
    }

  QCOMPARE(Protocol::add(object, v), result);
}

void TestProtocol::testDelet_data()
{
  QTest::addColumn<QString>("object");
  QTest::addColumn<QString>("name");

  QTest::newRow("validDeleteContact") << "Contact" << "name";
  QTest::newRow("without object") << "" << "name";
  QTest::newRow("without contact") << "Contact" << "";
}

void TestProtocol::testDelet()
{
  QFETCH(QString, object);
  QFETCH(QString, name);

  QString result = "{\"Delete" + object + "\": {\"Name\": \"" + name + "\"}}\n";
  QVector<QString> v;
  v.append(name);

  if (object == "")
    {
      result = "";
    }

  if (name == "")
    {
      v.remove(0);
      result = "";
    }

  QCOMPARE(Protocol::delet(object, v), result);
}
