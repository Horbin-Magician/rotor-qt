#ifndef POWERBOOT_H
#define POWERBOOT_H

#include <QApplication>
#include <QSettings>
#include <QFileInfo>
#include <QDir>


#define AUTO_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

class PowerBoot
{
public:
    /*******************************
     * 功能：设置/取消 进程开机自动启动函数
     * 参数：
        1、appPath：需要设置/取消的自启动软件的“绝对路径”
        2、flag：   设置/取消自启动标志位，1为设置，0为取消,默认为设置
    *******************************/
    static void setProcessAutoRun(bool flag = true)
    {
        QString appPath = QApplication::applicationFilePath();
        QSettings settings(AUTO_RUN, QSettings::NativeFormat);
        //以程序名称作为注册表中的键,根据键获取对应的值（程序路径）
        QFileInfo fInfo(appPath);
        QString name = fInfo.baseName();    //键-名称
        QString oldPath = settings.value(name).toString(); //获取目前的值-绝对路劲
        QString newPath = QDir::toNativeSeparators(appPath);    //toNativeSeparators函数将"/"替换为"\"
        if(flag){
            if(settings.contains(name) && newPath == oldPath) return;
            settings.setValue(name, newPath);
        }
        else
           settings.remove(name);
    }
};

#endif // POWERBOOT_H
