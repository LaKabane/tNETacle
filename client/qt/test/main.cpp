#include <QtTest>
#include "testcontroller.h"
#include "testcontact.h"
#include "testprotocol.h"

int main()
{
  TestController test1;
  TestContact test2;
  TestProtocol test3;
 
  QTest::qExec(&test1);
  QTest::qExec(&test2);
  QTest::qExec(&test3);
 
  return 0;
}
