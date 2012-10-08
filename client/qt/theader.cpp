#include "theader.h"

THeader::THeader(QWidget* parent) :
  QWidget(parent)
{
  this->setupUi(dynamic_cast<QFrame*>(this));
}

THeader::~THeader()
{
}
