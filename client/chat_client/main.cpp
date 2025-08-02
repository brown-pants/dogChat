#include "appinit.h"
#include "loginwnd.h"
#include "mainwnd.h"
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

    TcpClient::GetInstance();

    MainWnd mainwnd;
    mainwnd.hide();

    LoginWnd loginwnd;
    loginwnd.show();

    TcpClient::GetInstance().tcp_connect();

    qDebug() << "main thread: " << QThread::currentThread();

    return a.exec();
}
