#include <QtTest>
#include "testcontroller.h"
#include "testcontact.h"
#include "testprotocol.h"
#include "testnetwork.h"

int main()
{
  TestController test1;
  TestContact test2;
  TestProtocol test3;
  TestNetwork test4;
 
  QTest::qExec(&test1);
  QTest::qExec(&test2);
  QTest::qExec(&test3);
  QTest::qExec(&test4);
 
  return 0;
}
