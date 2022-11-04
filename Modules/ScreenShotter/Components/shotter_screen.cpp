#include "shotter_screen.h"

#include <QMenu>
#include <QMouseEvent>
#include <QApplication>
#include <QFileDialog>
#include <QClipboard>
#include <QDateTime>
#include <QPainter>
#include <windows.h>


ShotterScreen::ShotterScreen(std::shared_ptr<QPixmap> originPainting, QRect windowRect, QWidget *parent):QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow | Qt::WindowStaysOnTopHint);

    m_originPainting = originPainting->copy();
    m_isPressed = false;
    m_windowRect = windowRect;
    m_scaleRate = QGuiApplication::primaryScreen()->devicePixelRatio();
    m_zoom = 1;

    m_menu = new QMenu(this);
    m_menu->addAction(QStringLiteral("复制截图"), this, SLOT(onSaveScreen()));
    m_menu->addAction(QStringLiteral("保存"), this, SLOT(onSaveScreenOther()));
    m_menu->addAction(QStringLiteral("最小化"), this, SLOT(minimize()));
    m_menu->addAction(QStringLiteral("退出截图"), this, SLOT(quitScreenshot()));

    setGeometry(m_windowRect.x()/m_scaleRate, m_windowRect.y()/m_scaleRate, m_windowRect.width()/m_scaleRate, m_windowRect.height()/m_scaleRate);
    m_geoRect = geometry();

    setMouseTracking(true); // 开启鼠标实时追踪
    show();
}

// 判断鼠标区域
ShotterScreen::DIRECTION ShotterScreen::getRegion(const QPoint &cursor)
{
    ShotterScreen::DIRECTION ret_dir = NONE;

    QPoint pt_lu = rect().topLeft(); // left upper
    QPoint pt_rl = rect().bottomRight(); // right lower

    int x = cursor.x();
    int y = cursor.y();

    /// 获得鼠标当前所处窗口的边界方向
    if(pt_lu.x() + M_PADDING >= x
    && pt_lu.x() <= x
    && pt_lu.y() + M_PADDING >= y
    && pt_lu.y() <= y) {
        ret_dir = LEFTUPPER; // 左上角
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if(x >= pt_rl.x() - M_PADDING
           && x <= pt_rl.x()
           && y >= pt_rl.y() - M_PADDING
           && y <= pt_rl.y()) {
        ret_dir = RIGHTLOWER; // 右下角
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if(x <= pt_lu.x() + M_PADDING
           && x >= pt_lu.x()
           && y >= pt_rl.y() - M_PADDING
           && y <= pt_rl.y()) {
        ret_dir = LEFTLOWER; // 左下角
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if(x <= pt_rl.x()
           && x >= pt_rl.x() - M_PADDING
           && y >= pt_lu.y()
           && y <= pt_lu.y() + M_PADDING) {
        ret_dir = RIGHTUPPER; // 右上角
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if(x <= pt_lu.x() + M_PADDING
           && x >= pt_lu.x()) {
        ret_dir = LEFT; // 左边
        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if( x <= pt_rl.x()
            && x >= pt_rl.x() - M_PADDING) {
        ret_dir = RIGHT; // 右边
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }else if(y >= pt_lu.y()
          && y <= pt_lu.y() + M_PADDING){
        ret_dir = UPPER; // 上边
        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else if(y <= pt_rl.y()
           && y >= pt_rl.y() - M_PADDING) {
        ret_dir = LOWER; // 下边
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }else {
        ret_dir = NONE; // 默认
        this->setCursor(QCursor(Qt::SizeAllCursor));
    }
    return ret_dir;
}

// 在鼠标位置弹射出菜单栏
void ShotterScreen::contextMenuEvent(QContextMenuEvent *)
{
    m_menu->exec(cursor().pos());
}

void ShotterScreen::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_isPressed = true;
        m_movePos = e->position().toPoint();
    }
}

void ShotterScreen::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) m_isPressed = false;
}

void ShotterScreen::mouseMoveEvent(QMouseEvent *e)
{
    if(!m_isPressed) m_direction = getRegion(e->pos());
    if(m_isPressed) {
        if(m_direction != NONE) {
            // 鼠标进行拖拉拽
            QPointF globalPosition = e->globalPosition();
            QRectF geo = geometry();
            switch(m_direction) {
                case LEFT:
                    geo.setLeft(e->globalPosition().x());
                    break;
                case RIGHT:
                    geo.setRight(globalPosition.x());
                    break;
                case UPPER:
                    geo.setTop(globalPosition.y());
                    break;
                case LOWER:
                    geo.setBottom(globalPosition.y());
                    break;
                case LEFTUPPER:
                    geo.setTopLeft(globalPosition.toPoint());
                    break;
                case RIGHTUPPER:
                    geo.setTopRight(globalPosition.toPoint());
                    break;
                case LEFTLOWER:
                    geo.setBottomLeft(globalPosition.toPoint());
                case RIGHTLOWER:
                    geo.setBottomRight(globalPosition.toPoint());
                    break;
                default: break;
            }
            QRectF tmpRect = zoomRect(geo, 1/m_zoom);
            float x = m_windowRect.x() + (tmpRect.x() - m_geoRect.x()) * m_scaleRate / m_zoom;
            float y = m_windowRect.y() + (tmpRect.y() - m_geoRect.y()) * m_scaleRate / m_zoom;
            float width = tmpRect.width() * m_scaleRate;
            float height = tmpRect.height() * m_scaleRate;
            m_geoRect = zoomRect(geo, 1/m_zoom);
            m_windowRect.setRect(x, y, width, height);
            setGeometry(geo.toRect());
            update();
        }
        else {
            const QPoint delta = e->position().toPoint() - m_movePos;
            move(pos() + delta);
            m_geoRect.moveTo(m_geoRect.topLeft() + delta);
        }
    }
}

void ShotterScreen::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen = painter.pen();
    pen.setColor(QColor(0,175,255));
    pen.setWidth(2);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);
    painter.drawPixmap(rect(), m_originPainting, m_windowRect); // 绘制截屏编辑窗口
    painter.drawRect(rect()); // 绘制边框线
}

void ShotterScreen::wheelEvent(QWheelEvent *e)
{
    if(e->angleDelta().y() > 0){
        m_zoom = m_zoom + 0.1;
    }else if (e->angleDelta().y() < 0 && m_zoom > 0.1){
        m_zoom = m_zoom - 0.1;
    }
    setGeometry(zoomRect(m_geoRect, m_zoom).toRect());
}

void ShotterScreen::changeEvent(QEvent *event)
{
    if(QEvent::WindowStateChange == event->type()){
        QWindowStateChangeEvent * stateEvent = dynamic_cast<QWindowStateChangeEvent*>(event);
        if(Q_NULLPTR != stateEvent){
            if(Qt::WindowMinimized == stateEvent->oldState()){
                setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow | Qt::WindowStaysOnTopHint); // 设置成无边框对话框
                show();
            }
        }
    }
}

// 根据当前时间获得截图名
const QString ShotterScreen::getFileName()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString file_name = "Rotor_" + currentTime.toString(QStringLiteral("yyyy-MM-dd-HH-mm-ss"));
    return file_name;
}

void ShotterScreen::minimize()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint); // 设置成无边框对话框
    show();
    setWindowState(Qt::WindowMinimized);
}

QRectF ShotterScreen::zoomRect(const QRectF &rect, float zoom)
{
    return QRectF(rect.x(), rect.y(), rect.width()*zoom, rect.height()*zoom);
}

// 保存图片到剪切板
void ShotterScreen::onSaveScreen()
{
    QClipboard *board = QApplication::clipboard();
    board->setPixmap(m_originPainting.copy(m_windowRect.toRect())); // 把图片放入剪切板
}

// 保存图片到其他地方
void ShotterScreen::onSaveScreenOther()
{
    QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("保存图片"), getFileName(), "PNG Files (*.PNG)");
    if (fileName.length() > 0) {
        QPixmap pic = m_originPainting.copy(m_windowRect.toRect());
        pic.save(fileName, "png");
    }
}

// TODO 结束截图 (通知shotter)
void ShotterScreen::quitScreenshot(){
    close();
}
