#ifndef SCREENSHOTTER_H
#define SCREENSHOTTER_H

#include <memory>
#include <QWidget>
#include <QMenu>
#include <QRect>
#include <QList>

#include "../i_module.h"


class ShotterWindow;
class Amplifier;
class QTimer;
class QMenu;

/**
 * @brief The ScreenShotter class
 */
class ScreenShotter : public QWidget, IModule
{
    Q_OBJECT
signals:
    void cursorPosChange(int, int);

public:
    ScreenShotter(QWidget *parent = nullptr);

    virtual void onHotkey();
    void Shot();

protected:
    bool event(QEvent *);
    void paintEvent(QPaintEvent *);

private:
    unsigned short m_state;

    float m_scaleRate;
    QRect m_desktopRect; // 当前桌面屏幕的矩形数据
    std::shared_ptr<QPixmap> m_backgroundScreen; // 屏幕暗色背景图
    std::shared_ptr<QPixmap> m_originPainting; // 屏幕原画

    QList<ShotterWindow*> m_ShotterWindowList;
    std::shared_ptr<Amplifier> m_amplifierTool; // 放大取色器

    QPoint m_startPoint; // 用于检测误操作
    QRectF m_windowRect; // 当前鼠标选区最小的矩形窗口
    QTimer* m_egoisticTimer; // 置顶定时器

    void CaptureScreen();
    void initWindow(); // 初始化窗口，尺寸
    void initAmplifier(); // 初始化放大镜 (色彩采集器)
    void initMeasureWidget(void); // 测量控件 (大小感知器)
    void updateMouseWindow(void);
    void endShot();

    void onShotterWindowClose(ShotterWindow*);
    void onShotterWindowMove(ShotterWindow*);
};

#endif // SCREENSHOTTER_H


