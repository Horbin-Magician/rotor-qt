#include "screen_shotter.h"

#include <QGuiApplication>
#include <QApplication>
#include <QMouseEvent>
#include <QFileDialog>
#include <QClipboard>
#include <QDateTime>
#include <QPainter>
#include <QScreen>
#include <QCursor>
#include <QMutex>
#include <QPen>

#include <windows.h>

#include "Components/amplifier.h"
#include "Components/shotter_screen.h"


ScreenShotter::ScreenShotter(QWidget *parent) : QWidget(parent)
{
    m_state = 0;
    m_backgroundScreen = nullptr;
    m_originPainting = nullptr;
    m_desktopRect = QGuiApplication::primaryScreen()->geometry(); // 获取设备屏幕大小
    m_scaleRate = QGuiApplication::primaryScreen()->devicePixelRatio();

    initWindow(); // 初始化窗口

    RegisterHotKey((HWND)this->winId(),1, 0, (UINT)0x70); // 注册快捷键
}

// 析构函数，释放快捷键
ScreenShotter::~ScreenShotter(void) {
    UnregisterHotKey((HWND)this->winId(),1); // 释放快捷键
}

// 开始截图
void ScreenShotter::Shot()
{
    m_state = 1;
    CaptureScreen(); // 捕获屏幕
    initAmplifier(); // 初始化鼠标放大器
    emit cursorPosChange(cursor().pos().x(), cursor().pos().y()); // 更新鼠标的位置
    updateMouseWindow(); // 更新鼠标区域窗口
    show(); // 展示窗口
    this->setFocus();
}

bool ScreenShotter::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    if(eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        if(msg->message == WM_HOTKEY) {
            // deal with hotkey
            UINT fuModifiers = (UINT) LOWORD(msg->lParam);
            UINT uVirtKey = (UINT) HIWORD(msg->lParam);
            if(fuModifiers == (UINT)0 && uVirtKey == (UINT)0x70){
                this->Shot();
            }
        }
    }
    return QWidget::nativeEvent(eventType, message, result);
}

// 捕获屏幕
void ScreenShotter::CaptureScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen(); // 截取当前桌面，作为截屏的背景图
    // 抓取原始屏幕
    m_originPainting.reset(new QPixmap(screen->grabWindow(0, m_desktopRect.x(), m_desktopRect.y(), m_desktopRect.width(),m_desktopRect.height())));
    // 制作暗色屏幕背景
    QPixmap temp_dim_pix(m_originPainting->width(), m_originPainting->height());
    temp_dim_pix.fill((QColor(0, 0, 0, 160)));
    m_backgroundScreen.reset(new QPixmap(*m_originPainting));
    QPainter p(m_backgroundScreen.get());
    p.drawPixmap(0, 0, temp_dim_pix);
}

// 更新鼠标区域窗口
void ScreenShotter::updateMouseWindow()
{
    // 获得当前鼠标位置
    POINT pt;
    ::GetCursorPos(&pt);
    if (m_state == 1){
        ::EnableWindow((HWND)winId(), FALSE);
        // 获得当前位置桌面上的子窗口
        HWND hwnd = ::ChildWindowFromPointEx(::GetDesktopWindow(), pt, CWP_SKIPDISABLED | CWP_SKIPINVISIBLE);
        if (hwnd != NULL) {
            HWND temp_hwnd;
            temp_hwnd = hwnd;
            while (true) {
                ::GetCursorPos(&pt);
                ::ScreenToClient(temp_hwnd, &pt);
                temp_hwnd = ::ChildWindowFromPointEx(temp_hwnd, pt, CWP_SKIPINVISIBLE);
                if (temp_hwnd == NULL || temp_hwnd == hwnd) break;
                hwnd = temp_hwnd;
            }
            RECT temp_window;
            ::GetWindowRect(hwnd, &temp_window);
            m_windowRect.setRect(temp_window.left,temp_window.top, temp_window.right - temp_window.left, temp_window.bottom - temp_window.top);
        }
        ::EnableWindow((HWND)winId(), TRUE);
    }else if (m_state == 2){
        const int& rx = (pt.x >= m_startPoint.x()) ? m_startPoint.x() : pt.x;
        const int& ry = (pt.y >= m_startPoint.y()) ? m_startPoint.y() : pt.y;
        const int& rw = abs(pt.x - m_startPoint.x());
        const int& rh = abs(pt.y - m_startPoint.y());
        m_windowRect.setRect(rx, ry, rw, rh); // 改变大小
    }
    m_amplifierTool->onSizeChange(m_windowRect.width(), m_windowRect.height());
}

// 初始化窗口
void ScreenShotter::initWindow()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow);
    showFullScreen(); // 全屏窗口
    setGeometry(m_desktopRect); // 窗口与显示屏对齐
    setMouseTracking(true); // 开启鼠标实时追踪
    hide();
}

// 初始化取色器
void ScreenShotter::initAmplifier()
{
    m_amplifierTool.reset(new Amplifier(m_originPainting, this));
    connect(this,SIGNAL(cursorPosChange(int,int)), m_amplifierTool.get(), SLOT(onPostionChange(int,int)));
    m_amplifierTool->show();
    m_amplifierTool->raise();
}

// 结束截图
void ScreenShotter::endShot()
{
    m_amplifierTool->hide(); // 隐藏放大器
    this->hide();
    m_state = 0;
}

// 绘制背景和选区
void ScreenShotter::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    // 绘制背景
    painter.drawPixmap(0, 0, m_desktopRect.width(), m_desktopRect.height(), *m_backgroundScreen);
    // 绘制选区
    if (!m_windowRect.isEmpty()) {
        QPen pen = painter.pen();
        pen.setColor(QColor(0,175,255));
        pen.setWidth(4);
        pen.setJoinStyle(Qt::MiterJoin);
        painter.setPen(pen);
        float x = m_windowRect.x()  / m_scaleRate;
        float y = m_windowRect.y()  / m_scaleRate;
        float width = m_windowRect.width() / m_scaleRate;
        float height = m_windowRect.height() / m_scaleRate;
        QRectF scaledRect = QRectF(x, y, width, height);
        painter.drawPixmap(QPointF(x, y), *m_originPainting, m_windowRect); // 绘制截屏编辑窗口
        painter.drawRect(scaledRect); // 绘制边框线
    }
}

// Esc键退出截图
void ScreenShotter::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape) endShot();
    else e->ignore();
}

// 按下左键，新建截图窗口
void ScreenShotter::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_startPoint = QPoint(e->pos().x() * m_scaleRate, e->pos().y() * m_scaleRate);
        m_state = 2;
    }
}

void ScreenShotter::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_state == 2 && e->button() == Qt::LeftButton) {
        ShotterScreen* screen = new ShotterScreen(m_originPainting, m_windowRect);
        m_screenToolList.append(screen);
        endShot(); // 结束截图
    }
}

// 鼠标移动事件
void ScreenShotter::mouseMoveEvent(QMouseEvent *e) {
    emit cursorPosChange(e->position().x(), e->position().y());
    updateMouseWindow(); // 更新当前鼠标选中的窗口
    update();
}
