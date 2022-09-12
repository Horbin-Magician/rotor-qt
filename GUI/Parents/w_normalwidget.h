#ifndef NORMALWIDGET_H
#define NORMALWIDGET_H

#include <QWidget>

class NormalWidget : public QWidget
{
    Q_OBJECT
public:
    NormalWidget();
    void paintEvent(QPaintEvent* event) override;
};

#endif // NORMALWIDGET_H
