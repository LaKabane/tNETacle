#include <QtGui>
#include <QtTest/QtTest>

class TestProtocol : public QObject
{
  Q_OBJECT
  private slots:
  void testAdd_data();
  void testAdd();

  void testDelet_data();
  void testDelet();
};
