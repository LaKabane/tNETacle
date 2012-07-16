#include <QtGui>
#include <QtTest/QtTest>

class TestNetwork : public QObject
{
  Q_OBJECT
  private slots:
  void testIsConnected();
};
