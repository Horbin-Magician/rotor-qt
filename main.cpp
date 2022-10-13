#include <QApplication>
#include <windows.h>

#include "Utils/UAC.h"
#include "rotor.h"

// return: true, if already run a Rotor
bool checkAreadyRun()
{
    // create mutex
    HANDLE m_hMutex = CreateMutex(NULL, FALSE,  L"Manager" );
    // if get already_exists error, return true
    if(GetLastError() == ERROR_ALREADY_EXISTS){
        CloseHandle(m_hMutex);
        return true;
    }
    return false;
}

// entrance of total programe
int main(int argc, char *argv[])
{
    // check
    if ( checkAreadyRun() | UAC::runAsAdmin() ) return 0;

    // init Application
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    Rotor& rotor = Rotor::getInstance();


    return app.exec();
}
