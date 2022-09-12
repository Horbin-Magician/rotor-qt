#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include "GUI/w_searcher.h"
#include "GUI/w_setting.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QApplication>


class Manager : public QSystemTrayIcon
{
    Q_OBJECT
public:
    static Manager& getInstance();
    Searcher* getSearcher();
protected:
    QAction* addAction(QString name);
    void activeTray(QSystemTrayIcon::ActivationReason reason);
    void showMenu();//显示菜单
private:
    explicit Manager(QObject * parent = nullptr);
    ~Manager();

    QMenu* m_menu;
    QAction* m_action1;
    QAction* m_action2;
    QAction* m_action3;
    Searcher* m_searcher;
    Setting* m_setting;
};
#endif // SYSTEMTRAY_H
