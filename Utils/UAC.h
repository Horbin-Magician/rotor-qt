#ifndef UAC_H
#define UAC_H

#include <QCoreApplication>
#include <shlobj.h>


#pragma comment (lib, "Shell32.lib")

class UAC
{
public:
    // 以管理员权限启动一个新实例，true-启动了新实例，false-未启动新实例
    static bool runAsAdmin()
    {
        if (IsUserAnAdmin()) return false; // 当前程序正以管理员权限运行

        QStringList args = QCoreApplication::arguments(); // 获取命令行参数
        QString filePath = QCoreApplication::applicationFilePath();

        std::wstring temp=filePath.toStdWString();
        LPCWSTR filePath_w = temp.c_str();

        HINSTANCE ins = ShellExecuteW(nullptr, L"runas", filePath_w, L"runas", nullptr, SW_SHOWNORMAL);

        if (ins > (HINSTANCE)32) return true; // 程序新实例启动成功
        MessageBoxW(NULL,filePath_w,L"请以管理员身份运行！",MB_OK);
        return false;
    }
};

#endif // UAC_H
