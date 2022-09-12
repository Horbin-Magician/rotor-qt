#include "Utils/UAC.h"
#include "Model/settingmodel.h"
#include "manager.h"

#include <QApplication>
#include <windows.h>

bool checkOnly()
{
    //创建互斥量
    HANDLE m_hMutex  =  CreateMutex(NULL, FALSE,  L"Manager" );
    //检查错误代码
    if(GetLastError() == ERROR_ALREADY_EXISTS){
        CloseHandle(m_hMutex);
        return  false;
    }
    else
        return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv); // 初始化Application
    app.setQuitOnLastWindowClosed(false);

    if(!checkOnly() | UAC::runAsAdmin()) return 0; //检查是否已启动，并以管理员权限启动一个新实例

    Manager& manager = Manager::getInstance();
    Manager::getInstance();
    SettingModel& settingModel = SettingModel::getInstance();

    return app.exec();
}
