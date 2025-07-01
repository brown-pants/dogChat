#include "appinit.h"
#include <QApplication>
#include <QMouseEvent>
#include <QWidget>
#include <QFile>

void AppInit::Init()
{
    static bool isInited = false;
    static AppInit appInit;

    if (isInited)
    {
        qDebug() << "程序重复初始化" << Qt::endl;
        return;
    }

    // 加载样式表
    QFile file(":/res/qss/StyleSheet.css");
    if (file.open(QFile::ReadOnly))
    {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    }
    else
    {
        qDebug() << "样式表加载失败" << Qt::endl;
    }


    // 注册全局事件过滤器
    qApp->installEventFilter(&appInit);
}

bool AppInit::eventFilter(QObject *obj, QEvent *event)
{
    static int resizeArea = ResizeNone;
    static bool lPressed = false;
    static QPointF mouseGlobalPos;

    QWidget *w = (QWidget *)obj;
    QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);

    // 获取对象属性
    bool canMove = w->property("canMove").toBool();
    bool canResize = w->property("canResize").toBool();

    // 鼠标移动到列表时，改变光标为默认箭头
    if (event->type() == QEvent::MouseMove && (obj->inherits("ChatListItem") || obj->inherits("QListWidget")))
    {
        w->setCursor(Qt::ArrowCursor);
    }

    // 无移动属性和伸缩属性
    if (!canMove && !canResize)
    {
        return QObject::eventFilter(obj, event);
    }

    // 事件处理
    if (event->type() == QEvent::MouseButtonPress && mouse_event->button() == Qt::LeftButton)
    {
        lPressed = true;
        mouseGlobalPos = mouse_event->globalPosition();
    }
    else if (event->type() == QEvent::MouseButtonRelease && mouse_event->button() == Qt::LeftButton)
    {
        lPressed = false;
        w->setCursor(Qt::ArrowCursor);
    }
    else if (event->type() == QEvent::MouseMove)
    {
#if defined(__unix__)
        w->update();
#endif
        int shadowWidth = w->property("shadowWidth").toInt();
        if (lPressed)
        {
            // 鼠标在边缘区域，则调整窗口大小
            if (canResize && resizeArea != ResizeNone)
            {
                QRect newGeometry = w->geometry();
                QPointF mouseGlobPos = mouse_event->globalPosition();

                if (resizeArea & ResizeLeft)
                {
                    int newWidth = w->geometry().right() - mouseGlobPos.x();
                    if (newWidth >= w->minimumWidth() && newWidth <= w->maximumWidth())
                    {
                        newGeometry.setLeft(mouseGlobPos.x() - shadowWidth);
                    }
                }
                if (resizeArea & ResizeRight)
                {
                    newGeometry.setRight(mouseGlobPos.x() + shadowWidth);
                }
                if (resizeArea & ResizeTop)
                {
                    int newHeight = w->geometry().bottom() - mouseGlobPos.y();
                    if (newHeight >= w->minimumHeight() && newHeight <= w->maximumHeight())
                    {
                        newGeometry.setTop(mouseGlobPos.y() - shadowWidth);
                    }
                }
                if (resizeArea & ResizeBottom)
                {
                    newGeometry.setBottom(mouseGlobPos.y() + shadowWidth);
                }

                w->setGeometry(newGeometry);
            }
            // 否则移动窗口
            else if (canMove)
            {
                QPoint offset = (mouse_event->globalPosition() - mouseGlobalPos).toPoint();
                w->move(w->pos() + offset);
                mouseGlobalPos = mouse_event->globalPosition();
            }
        }
        else if (canResize)
        {
            // 检测鼠标是否在边缘区域
            const int borderOffset = shadowWidth - 5;
            const int borderWidth = shadowWidth + 5;
            QPoint mousePos = mouse_event->pos();
            resizeArea = ResizeNone;

            if (mousePos.x() > borderOffset                 && mousePos.x() < borderWidth)                  resizeArea |= ResizeLeft;
            if (mousePos.y() > borderOffset                 && mousePos.y() < borderWidth)                  resizeArea |= ResizeTop;
            if (mousePos.x() < w->width() - borderOffset    && mousePos.x() > w->width() - borderWidth)     resizeArea |= ResizeRight;
            if (mousePos.y() < w->height() - borderOffset   && mousePos.y() > w->height() - borderWidth)    resizeArea |= ResizeBottom;

            // 设置适当的光标形状
            if ((resizeArea & ResizeLeft && resizeArea & ResizeTop) || (resizeArea & ResizeRight && resizeArea & ResizeBottom))
            {
                w->setCursor(Qt::SizeFDiagCursor);
            }
            else if ((resizeArea & ResizeLeft && resizeArea & ResizeBottom) || (resizeArea & ResizeRight && resizeArea & ResizeTop))
            {
                w->setCursor(Qt::SizeBDiagCursor);
            }
            else if (resizeArea & (ResizeLeft | ResizeRight))
            {
                w->setCursor(Qt::SizeHorCursor);
            }
            else if (resizeArea & (ResizeTop | ResizeBottom))
            {
                w->setCursor(Qt::SizeVerCursor);
            }
            else
            {
                w->setCursor(Qt::ArrowCursor);
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

AppInit::AppInit(QObject *parent)
    : QObject{parent}
{}
