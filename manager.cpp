#include "manager.h"


Manager& Manager::getInstance()
{
    static Manager m_pInstance;
    return m_pInstance;
}

Searcher *Manager::getSearcher()
{
    return m_searcher;
}

Manager::Manager(QObject *parent)
{
    m_searcher = new Searcher();
    m_setting = new Setting();
    connect(m_setting, &Setting::rebuildIndex, m_searcher, &Searcher::initFileData);

    m_menu = new QMenu();
    connect(addAction("设置"), &QAction::triggered, m_setting, &Setting::show);
    connect(addAction("退出"), &QAction::triggered, this, &QApplication::quit);

    this->setContextMenu(m_menu);
    this->setIcon(QIcon(":/picture/Resources/favicon.ico"));
    this->setToolTip("小云管家");
    connect(this, &QSystemTrayIcon::activated, this, &Manager::activeTray);  //点击托盘，执行相应的动作

    this->show();
}

Manager::~Manager()
{
    delete m_menu;
}

QAction* Manager::addAction(QString name)
{
    QAction* action = new QAction(m_menu);
    action->setText(name);
    m_menu->addAction(action);
    return action;
}

void Manager::activeTray(QSystemTrayIcon::ActivationReason reason)
{
    // deal tray event
    switch (reason)
    {
        case QSystemTrayIcon::Context: showMenu();break; // 右键显示菜单
        case QSystemTrayIcon::Trigger: m_searcher->show();break; // 点击显示窗口
        default: break;
    }
}

void Manager::showMenu()
{
    m_menu->show();
}
