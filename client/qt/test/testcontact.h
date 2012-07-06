#include <QtGui>
#include <QtTest/QtTest>

class TestConfig : public QObject
{
  Q_OBJECT
  private slots:
  void testChange_data();
  void testChange();

  void testRead_data();
  void testRead();

  void testSave_data();
  void testSave();
};
