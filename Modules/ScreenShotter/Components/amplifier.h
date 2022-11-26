#ifndef OEAMPLIFIER_H
#define OEAMPLIFIER_H
#include <memory>
#include <QWidget>


// 放大取色器，关于鼠标位置局部放大的操作以及色卡取值的操作
class Amplifier : public QWidget
{
    Q_OBJECT
public:
    explicit Amplifier(std::shared_ptr<QPixmap> originPainting, QWidget *parent = 0);

    int switchColorType();
    QString getColorStr();

public slots:
    void onSizeChange(int w, int h); // 大小修改
    void onPostionChange(int x, int y); // 位置修改
protected:
    virtual void paintEvent(QPaintEvent *); // 窗口自绘事件

private:
    float m_scaleRate;
    QSize m_screenSize; // 外部组件的大小信息
    QPoint m_cursorPoint; // 鼠标的位置
    int m_sideLength; // 取色放大器的边长
    int m_imageHeight; // 放大区的高度
    std::shared_ptr<QPixmap> m_originPainting; // 屏幕原画

    int m_colorType;
    QString m_colorStr;
};

#endif /// OEAMPLIFIER_H
