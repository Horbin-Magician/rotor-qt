#ifndef UAC_H
#define UAC_H

#include <QCoreApplication>
#include <shlobj.h>

#pragma comment (lib, "Shell32.lib")


class UAC
{
public:
    // run a programe as admin
    // return: true, if run successfully
    static bool runAsAdmin()
    {
        if (IsUserAnAdmin()) return false;

        QStringList args = QCoreApplication::arguments();
        QString filePath = QCoreApplication::applicationFilePath();

        std::wstring temp=filePath.toStdWString();
        LPCWSTR filePath_w = temp.c_str();

        HINSTANCE ins = ShellExecuteW(nullptr, L"runas", filePath_w, L"runas", nullptr, SW_SHOWNORMAL);

        if (ins > (HINSTANCE)32) return true; // return true if programe run successfully
        MessageBoxW(NULL, L"该软件需要在管理员权限下建立索引。", L"请以管理员身份运行",MB_OK);
        return false;
    }
};

#endif // UAC_H
