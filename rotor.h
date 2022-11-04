#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QApplication>

#include "Modules/Searcher/searcher.h"
#include "Modules/ScreenShotter/screen_shotter.h"
#include "GUI/w_setting.h"
#include "Utils/hotkey.h"

// the main class of Rotor, control all the programe
class Rotor : public QSystemTrayIcon
{
    Q_OBJECT
public:
    static Rotor& getInstance();
    Searcher* getSearcher();
private:
    explicit Rotor(QObject * parent = nullptr);

    QAction* addAction(QString name);
    void activeTray(QSystemTrayIcon::ActivationReason reason);

    QMenu* m_menu;
    Searcher* m_searcher; // searcher module
    ScreenShotter* m_screenShotter; // screen_shtter module
    HotKey* m_hotkey;

    Setting* m_setting; // setting window
};
#endif // SYSTEMTRAY_H
