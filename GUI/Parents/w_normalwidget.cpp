#include "w_normalwidget.h"

NormalWidget::NormalWidget()
{
    this->setMouseTracking(true);
}

void NormalWidget::paintEvent(QPaintEvent *event)
{
//    opt = QStyleOption()
//    opt.initFrom(self)
//    painter = QPainter(self)
//    self.style().drawPrimitive(QStyle.PE_Widget, opt, painter, self)
    QWidget::paintEvent(event);
}
