#include <QtTest>
#include "testcontroller.h"
#include "testcontact.h"

int main()
{
  TestController test1;
  TestContact test2;
 
  QTest::qExec(&test1);
  QTest::qExec(&test2);
 
  return 0;
}
