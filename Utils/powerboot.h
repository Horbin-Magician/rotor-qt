#ifndef POWERBOOT_H
#define POWERBOOT_H

#include <QApplication>
#include <QSettings>
#include <QFileInfo>
#include <QDir>


class PowerBoot
{
public:
    // set or cancel powerboot
    // input: flag, true to set and false to cancel
    static void setProcessAutoRun(bool flag = true)
    {
        QString auto_run_key = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        QString programe_key = "Rotor";

        QString appPath = QApplication::applicationFilePath();
        QSettings settings(auto_run_key, QSettings::NativeFormat);

        QString oldPath = settings.value(programe_key).toString(); // get value of programe_key
        QString newPath = QDir::toNativeSeparators(appPath); // replace '/' with '\'
        if(flag){
            if(settings.contains(programe_key) && newPath == oldPath) return;
            settings.setValue(programe_key, newPath);
        }
        else{
            settings.remove(programe_key);
        }
    }
};

#endif // POWERBOOT_H
