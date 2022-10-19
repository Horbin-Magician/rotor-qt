#ifndef SCREENSHOTTER_H
#define SCREENSHOTTER_H

#include <memory>
#include <QWidget>
#include <QMenu>
#include <QRect>
#include <QList>

class ShotterScreen;
class Amplifier;
class QTimer;
class QMenu;

/**
 * @brief The ScreenShotter class
 */
class ScreenShotter : QWidget
{
    Q_OBJECT
signals:
    void cursorPosChange(int, int);

public:
    ScreenShotter(QWidget *parent = nullptr);
    ~ScreenShotter();

    void Shot();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *);

private:
    unsigned short m_state;

    float m_scaleRate;
    QRect m_desktopRect; // 当前桌面屏幕的矩形数据
    std::shared_ptr<QPixmap> m_backgroundScreen; // 屏幕暗色背景图
    std::shared_ptr<QPixmap> m_originPainting; // 屏幕原画

    QList<ShotterScreen*> m_screenToolList;
    std::shared_ptr<Amplifier> m_amplifierTool; // 放大取色器

    QPoint m_startPoint; // 用于检测误操作
    QRect m_windowRect; // 当前鼠标选区最小的矩形窗口
    QTimer* m_egoisticTimer; // 置顶定时器

    void CaptureScreen();
    void initWindow(); // 初始化窗口，尺寸
    void initAmplifier(); // 初始化放大镜 (色彩采集器)
    void initMeasureWidget(void); // 测量控件 (大小感知器)
    void updateMouseWindow(void);
    void endShot();
};

#endif // SCREENSHOTTER_H


