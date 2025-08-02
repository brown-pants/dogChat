#include "appinit.h"
#include "loginwnd.h"
#include "mainwnd.h"
#include "storagemanager.h"
#include "tcpclient.h"

#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
#if defined(__unix__)
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

    QApplication a(argc, argv);
    AppInit::Init();

    StorageManager::GetInstance();
    TcpClient::GetInstance();

    MainWnd mainwnd;
    mainwnd.hide();

    LoginWnd loginwnd;
    loginwnd.show();

    TcpClient::GetInstance().tcp_connect();

    qDebug() << "main thread: " << QThread::currentThread();

    return a.exec();
}
