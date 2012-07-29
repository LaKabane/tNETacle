#include <QtGui>
#include <QtTest/QtTest>

class TestContact : public QObject
{
  Q_OBJECT
  private slots:

  void testAddContact_data();
  void testAddContact();
};
