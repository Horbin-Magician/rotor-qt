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
#include <dwmapi.h>

#include "Components/amplifier.h"
#include "Components/shotter_window.h"

ScreenShotter::ScreenShotter(QWidget *parent) : QWidget(parent)
{
    m_state = 0;
    m_isHidden = false;
    m_backgroundScreen = nullptr;
    m_originPainting = nullptr;
    m_desktopRect = QGuiApplication::primaryScreen()->geometry(); // 获取设备屏幕大小
    m_scaleRate = QGuiApplication::primaryScreen()->devicePixelRatio();
    initWindow(); // 初始化窗口
}

void ScreenShotter::onHotkey(unsigned int fsModifiers, unsigned int  vk)
{
    if(m_state == 1)return;
    if(vk == (UINT)0x43) Shot();
    else if(vk == (UINT)0x48) HideAll();
}

// 开始截图
void ScreenShotter::Shot()
{
    m_state = 1;
    CaptureScreen(m_originPainting, m_backgroundScreen); // 捕获屏幕
    initAmplifier(); // 初始化鼠标放大器
    emit cursorPosChange(cursor().pos().x(), cursor().pos().y()); // 更新鼠标的位置
    updateMouseWindow(); // 更新鼠标区域窗口
    show(); // 展示窗口
    this->activateWindow();
    this->setFocus();
}

void ScreenShotter::HideAll()
{
    bool ifMinimize = false;
    foreach (ShotterWindow* win, m_ShotterWindowList) {
        if(win->windowState() != Qt::WindowMinimized){
            ifMinimize = true;
            break;
        }
    }
    foreach (ShotterWindow* win, m_ShotterWindowList) {
        if(ifMinimize == true) win->minimize();
        else win->setWindowState(Qt::WindowNoState);
    }
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
        pen.setWidth(2);
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

bool ScreenShotter::event(QEvent *e)
{
    if(e->type() == QEvent::KeyPress){
        QKeyEvent* keyEvent = (QKeyEvent*) e;
        if (keyEvent->key() == Qt::Key_Escape) endShot(); // Esc键退出截图
        else if (keyEvent->key() == Qt::Key_H) SwitcHideShotterWin();// 隐藏其他窗口
        else if (keyEvent->key() == Qt::Key_Z) m_amplifierTool->switchColorType(); // Z键切换颜色
        else if (keyEvent->key() == Qt::Key_C){ // C键复制颜色
            QString colorStr = m_amplifierTool->getColorStr();
            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setText(colorStr);
        }
        else keyEvent->ignore();
    } else if(e->type() == QEvent::MouseMove){
        QMouseEvent* mouseEvent = (QMouseEvent*) e;
        emit cursorPosChange(mouseEvent->position().x(), mouseEvent->position().y());
        updateMouseWindow(); // 更新当前鼠标选中的窗口
        update();
    } else if(e->type() == QEvent::MouseButtonPress){ // 按下左键，开始绘制截图窗口
        QMouseEvent* mouseEvent = (QMouseEvent*) e;
        if (mouseEvent->button() == Qt::LeftButton) {
            m_startPoint = QPoint(mouseEvent->pos().x() * m_scaleRate, mouseEvent->pos().y() * m_scaleRate);
            m_state = 2;
        }
    } else if(e->type() == QEvent::MouseButtonRelease){ // 松开左键，新建截图窗口
        QMouseEvent* mouseEvent = (QMouseEvent*) e;
        if (m_state == 2 && mouseEvent->button() == Qt::LeftButton) {
            ShotterWindow* shotterWindow = new ShotterWindow(m_originPainting, m_windowRect);
            connect(shotterWindow, &ShotterWindow::sgn_close, this, &ScreenShotter::onShotterWindowClose);
            connect(shotterWindow, &ShotterWindow::sgn_move, this, &ScreenShotter::onShotterWindowMove);
            m_ShotterWindowList.append(shotterWindow);
            endShot(); // 结束截图
        }
    }
    return QWidget::event(e);
}

// 捕获屏幕
void ScreenShotter::CaptureScreen(std::shared_ptr<QPixmap>& originPainting, std::shared_ptr<QPixmap>& backgroundScreen)
{
    QScreen *screen = QGuiApplication::primaryScreen(); // 截取当前桌面，作为截屏的背景图
    // 抓取原始屏幕
    originPainting.reset(new QPixmap(screen->grabWindow(0, m_desktopRect.x(), m_desktopRect.y(), m_desktopRect.width(),m_desktopRect.height())));
    // 制作暗色屏幕背景
    QPixmap temp_dim_pix(originPainting->width(), originPainting->height());
    temp_dim_pix.fill((QColor(0, 0, 0, 160)));
    backgroundScreen.reset(new QPixmap(*originPainting));
    QPainter p(backgroundScreen.get());
    p.drawPixmap(0, 0, temp_dim_pix);
}

// 更新鼠标区域窗口
void ScreenShotter::updateMouseWindow()
{
    POINT pt;
    ::GetCursorPos(&pt); // 获得当前鼠标位置
    if (m_state == 1){
        ::EnableWindow((HWND)winId(), FALSE);
        // 获得当前位置桌面上的子窗口
        HWND hwnd = ::ChildWindowFromPointEx(::GetDesktopWindow(), pt, CWP_SKIPDISABLED | CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
        RECT temp_window;
        ::DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &temp_window, sizeof(temp_window));
        m_windowRect.setRect(temp_window.left,temp_window.top, temp_window.right - temp_window.left, temp_window.bottom - temp_window.top);
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
    setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow | Qt::WindowStaysOnTopHint);
    setGeometry(m_desktopRect); // 窗口与显示屏对齐
    setMouseTracking(true); // 开启鼠标实时追踪
    setFocusPolicy(Qt::StrongFocus);
    this->setCursor(QCursor(Qt::CrossCursor));
    hide();
}

// 初始化取色器
void ScreenShotter::initAmplifier()
{
    m_amplifierTool.reset(new Amplifier(m_originPainting, this));
    connect(this, &ScreenShotter::cursorPosChange, m_amplifierTool.get(), &Amplifier::onPostionChange);
    m_amplifierTool->show();
    m_amplifierTool->raise();
}

// 结束截图
void ScreenShotter::endShot()
{
    m_amplifierTool->hide(); // 隐藏放大器
    this->hide();
    m_state = 0;
    foreach (ShotterWindow* win, m_ShotterWindowList) win->show();
    m_ShotterWindowList.last()->raise();
    m_isHidden = false;
}

void ScreenShotter::onShotterWindowClose(ShotterWindow * shotterWindow)
{
    m_ShotterWindowList.removeAll(shotterWindow);
}

void ScreenShotter::onShotterWindowMove(ShotterWindow * shotterWindow)
{
    foreach (ShotterWindow* otherWin, m_ShotterWindowList) {
        if(otherWin != shotterWindow){
            QRect rectA = shotterWindow->geometry();
            QRect rectB = otherWin->geometry();
            int padding = 10;

            if(!(rectA.top() > rectB.bottom()) && !(rectA.bottom() < rectB.top())){
                if( qAbs(rectA.right() - rectB.left()) < padding)
                    shotterWindow->stick(STICK_TYPE::RIGHT_LEFT, otherWin);
                else if( qAbs(rectA.right() - rectB.right()) < padding)
                    shotterWindow->stick(STICK_TYPE::RIGHT_RIGHT, otherWin);
                else if( qAbs(rectA.left() - rectB.right()) < padding)
                    shotterWindow->stick(STICK_TYPE::LEFT_RIGHT, otherWin);
                else if( qAbs(rectA.left() - rectB.left()) < padding)
                    shotterWindow->stick(STICK_TYPE::LEFT_LEFT, otherWin);
            }

            if(!(rectA.right() < rectB.left()) && !(rectA.left() > rectB.right())){
                if( qAbs(rectA.top() - rectB.bottom()) < padding)
                    shotterWindow->stick(STICK_TYPE::UPPER_LOWER, otherWin);
                else if( qAbs(rectA.bottom() - rectB.top()) < padding)
                    shotterWindow->stick(STICK_TYPE::LOWER_UPPER, otherWin);
                else if( qAbs(rectA.top() - rectB.top()) < padding)
                    shotterWindow->stick(STICK_TYPE::UPPER_UPPER, otherWin);
                else if( qAbs(rectA.bottom() - rectB.bottom()) < padding)
                    shotterWindow->stick(STICK_TYPE::LOWER_LOWER, otherWin);
            }
        }
    }
}

void ScreenShotter::SwitcHideShotterWin(){
    if(m_isHidden == false){
        endShot();
        foreach (ShotterWindow* win, m_ShotterWindowList) win->hide();
        m_isHidden = true;
        Shot();
    }
}

