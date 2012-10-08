#ifndef THEADER_H_
# define THEADER_H_

#include <QWidget>
#include "ui_theader.h"

namespace Ui {
    class THeader;
}

class THeader : public QWidget, Ui::THeader
{
    Q_OBJECT

public:
    explicit   THeader(QWidget* parent = 0);
    virtual    ~THeader();
};

#endif /* !THEADER_H_ */
