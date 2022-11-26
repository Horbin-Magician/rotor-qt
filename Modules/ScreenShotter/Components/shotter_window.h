#ifndef SHOTTERSCREEN_H
#define SHOTTERSCREEN_H

#include <QWidget>

enum DIRECTION {UPPER=0,LOWER,LEFT,RIGHT,LEFTUPPER,LEFTLOWER,RIGHTLOWER,RIGHTUPPER,NONE}; // 方位枚举
enum STICK_TYPE {UPPER_UPPER=0, UPPER_LOWER, LOWER_UPPER, LOWER_LOWER, LEFT_RIGHT, LEFT_LEFT, RIGHT_RIGHT, RIGHT_LEFT}; // 方位枚举

class ShotterWindow : public QWidget {
    Q_OBJECT
signals:
    void sgn_close(ShotterWindow*);
    void sgn_move(ShotterWindow*);

public:
    explicit ShotterWindow(std::shared_ptr<QPixmap> originPainting, QRectF windowRect, QWidget *parent = 0);
    ~ShotterWindow() {}

    void stick(STICK_TYPE, ShotterWindow*);

protected:
    void contextMenuEvent(QContextMenuEvent *e); // 呼出菜单事件
    void mousePressEvent(QMouseEvent *e); // 鼠标按下事件
    void mouseReleaseEvent(QMouseEvent *e); // 鼠标释放事件
    void mouseMoveEvent(QMouseEvent *e); // 鼠标移动事件
    void paintEvent(QPaintEvent *e); // 界面自绘事件
    void wheelEvent(QWheelEvent *e);
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
public slots:
    void onSaveScreen(void); // 保存屏幕到剪切板中
    void onSaveScreenOther(void); // 保存编辑屏幕到其他路径下
    void quitScreenshot(void); // 退出当前截图窗口
    void minimize();
private:
    const int M_PADDING = 6; // 内边距，决定拖拽的触发。

    bool m_isStickX;
    bool m_isStickY;

    float m_scaleRate;
    DIRECTION m_direction;
    QRectF m_geoRect; // 对应缩放前显示的虚拟窗口
    QRectF m_windowRect; // 对应图片区域的虚拟窗口
    float m_zoom; // 窗口缩放的数值
    bool m_isPressed; // 鼠标是否按下
    QPoint m_movePos; // 拖动的距离
    QPixmap m_originPainting; // 屏幕原画
    QMenu* m_menu; // 右键菜单对象
    QMap<DIRECTION, QList<ShotterWindow*>> m_stickMap; // 贴贴列表

    QRectF zoomRect(const QRectF& rect, float zoom);

    DIRECTION getMouseRegion(const QPoint &cursor); // 获得当前坐标点的边界方位
    virtual const QString getFileName(void); // 获得一个以时间格式命名的文件名

};

#endif // SHOTTERSCREEN_H
