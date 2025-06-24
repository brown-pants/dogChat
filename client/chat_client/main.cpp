#include "appinit.h"
#include "mainwnd.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AppInit::Init();

    MainWnd mainwnd;
    mainwnd.show();

    return a.exec();
}
