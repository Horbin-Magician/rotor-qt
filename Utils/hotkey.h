#ifndef HOTKEY_H
#define HOTKEY_H


#include <QWidget>
#include <windows.h>

#include "../Modules/i_module.h"


struct HotKeyItem
{
    int id;
    UINT fsModifiers;
    UINT vk;
    IModule* module;
};

class HotKey : public QWidget
{
    Q_OBJECT
public:
    explicit HotKey(QWidget *parent = nullptr){};
    ~HotKey(){
        foreach (HotKeyItem item, m_HotKeyList) {
            UnregisterHotKey((HWND)this->winId(), item.id);
        }
    };
    bool RgtHotKey(LPCTSTR id, UINT fsModifiers, UINT vk, IModule* module){
        int idAtm = GlobalAddAtom(id);
        BOOL t = RegisterHotKey((HWND)this->winId(), idAtm, fsModifiers, vk);
        if(t == false) return false;
        HotKeyItem hotkey;
        hotkey.id = idAtm;
        hotkey.fsModifiers = fsModifiers;
        hotkey.vk = vk;
        hotkey.module = module;
        m_HotKeyList.append(hotkey);
        return true;
    };
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result){
        if(eventType == "windows_generic_MSG") {
            MSG *msg = static_cast<MSG *>(message);
            if(msg->message == WM_HOTKEY) {
                // deal with hotkey
                UINT fuModifiers = (UINT) LOWORD(msg->lParam);
                UINT uVirtKey = (UINT) HIWORD(msg->lParam);

                foreach (HotKeyItem item, m_HotKeyList)
                    if(fuModifiers == item.fsModifiers & uVirtKey == item.vk)
                        item.module->onHotkey(fuModifiers, uVirtKey);
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    };
private:
    QList<HotKeyItem> m_HotKeyList;
};

#endif // HOTKEY_H
