#include "appinit.h"
#include "mainwnd.h"
#include "loginwnd.h"
#include "storagemanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#if defined(__unix__)
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

    QApplication a(argc, argv);
    AppInit::Init();

    MainWnd mainwnd;
    mainwnd.show();


    LoginWnd login;
    login.show();

    StorageManager::GetInstance();

    return a.exec();
}
