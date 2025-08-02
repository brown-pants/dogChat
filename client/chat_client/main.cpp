#include "appinit.h"
#include "loginwnd.h"
#include "mainwnd.h"
#include "TcpClient.h"
#include "storagemanager.h"

#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
#if defined(__unix__)
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

    QApplication a(argc, argv);
    AppInit::Init();

    TcpClient::GetInstance();

    // 初始时直接连接TCP服务器
    TcpClient::GetInstance().tcp_connect();

    MainWnd mainwnd;
    mainwnd.hide();

    LoginWnd loginwnd;
    loginwnd.show();

    qDebug() << "main thread: " << QThread::currentThread();

    return a.exec();
}
