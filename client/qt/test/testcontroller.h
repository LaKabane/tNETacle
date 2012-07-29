#include <QtGui>
#include <QtTest/QtTest>

class TestController : public QObject
{
  Q_OBJECT
  private slots:

  void testRootNode_data();
  void testRootNode();

  void testAddContact_data();
  void testAddContact();
};
