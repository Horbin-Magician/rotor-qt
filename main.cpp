#include <QApplication>
#include <windows.h>

#include "Utils/UAC.h"
#include "rotor.h"

// return: true, if already run a Rotor
bool checkAreadyRun(HANDLE &hMutex)
{
    // create mutex
    hMutex = CreateMutex(NULL, FALSE,  L"Rotor" );
    // if get already_exists error, return true
    if(GetLastError() == ERROR_ALREADY_EXISTS){
        CloseHandle(hMutex);
        hMutex = NULL;
        MessageBox(NULL, L"一山二虎！", L"提示", MB_OK);
        return true;
    }
    return false;
}

// entrance of total programe
int main(int argc, char *argv[])
{
    // init Application
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // check
    HANDLE hMutex = NULL;
    if ( checkAreadyRun(hMutex) | UAC::runAsAdmin() ) {
        if(hMutex != NULL){
            ReleaseMutex(hMutex);
            CloseHandle(hMutex);
        }
        return 0;
    }

    // init Rotor
    Rotor& rotor = Rotor::getInstance();

    return app.exec();
}
