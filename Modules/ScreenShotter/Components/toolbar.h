#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QPushButton>
#include <QEvent>
#include <QApplication>

class Toolbar : public QWidget
{
    Q_OBJECT
public:
    explicit Toolbar(QWidget *parent = nullptr);

    void movePosition(QRect);
signals:
    void sgn_complete();
    void sgn_save();
    void sgn_minimize();
    void sgn_close();

protected:
    bool event(QEvent *e);

private:
    void initUI();
    QHBoxLayout* m_layout;
};

#endif // TOOLBAR_H
