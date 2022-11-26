#include "rotor.h"
#include "Models/setting_model.h"
#include "Modules/i_module.h"

// return: instance of Rotor
Rotor& Rotor::getInstance()
{
    static Rotor m_pInstance;
    return m_pInstance;
}

Searcher *Rotor::getSearcher()
{
    return m_searcher;
}

Rotor::Rotor(QObject *parent)
{
    SettingModel& settingModel = SettingModel::getInstance();

    m_searcher = new Searcher();
    m_screenShotter = new ScreenShotter();

    m_setting = new Setting();

    m_hotkey = new HotKey();
    m_hotkey->RgtHotKey(L"seacher", MOD_SHIFT, (UINT)0x46, (IModule*) m_searcher);
    m_hotkey->RgtHotKey(L"screenShotter", MOD_SHIFT, (UINT)0x43, (IModule*) m_screenShotter);

    m_menu = new QMenu();
    connect(addAction("设置"), &QAction::triggered, m_setting, &Setting::show);
    connect(addAction("退出"), &QAction::triggered, this, &QApplication::exit, Qt::QueuedConnection);

    this->setContextMenu(m_menu);
    this->setIcon(QIcon(":/picture/Resources/favicon.ico"));
    this->setToolTip("小云管家\n搜索：Shift+F\n截图：Shift+C"); // TODO: auto edit the tips
    connect(this, &QSystemTrayIcon::activated, this, &Rotor::activeTray); // combine activated event
    this->show();
}

// input: the name of action
// return: an action, named and added to m_menu
QAction* Rotor::addAction(QString name)
{
    QAction* action = new QAction(m_menu);
    action->setText(name);
    m_menu->addAction(action);
    return action;
}

// deal with tray event
void Rotor::activeTray(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Context: m_menu->show();break; // right click show menu
        case QSystemTrayIcon::Trigger: m_searcher->SwitchShow();break; // click show searcher
        default: break;
    }
}
