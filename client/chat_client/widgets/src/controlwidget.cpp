#include "controlwidget.h"
#include "ui_controlwidget.h"
#include <QPixmap>
#include <QMutex>
#include <QScreen>

QMutex mutex;

ControlWidget::ControlWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlWidget), controlWnd(nullptr), isNormalState(true), normalIcon(":/res/img/Substract.png"), maximizeIcon(":/res/img/Square.png")
{
    ui->setupUi(this);
}

ControlWidget::~ControlWidget()
{
    delete ui;
}

void ControlWidget::setControlWnd(QWidget *w)
{
    controlWnd = w;
    controlWnd->installEventFilter(this);
}

bool ControlWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 控制窗口移动时还原大小                                  尝试上锁
    if (event->type() == QEvent::Move && !isNormalState && mutex.tryLock())
    {
        QSize maximizeSize = controlWnd->size();
        int shadowWidth = controlWnd->property("shadowWidth").toInt();
        QPoint cursorPos = QCursor::pos() + QPoint(shadowWidth, shadowWidth);

        double xRatio = static_cast<double>(cursorPos.x()) / maximizeSize.width();
        double yRatio = static_cast<double>(cursorPos.y()) / maximizeSize.height();

        int x = cursorPos.x() - static_cast<int>(xRatio * wndNormalGeometry.width());
        int y = cursorPos.y() - static_cast<int>(yRatio * wndNormalGeometry.height());

        controlWnd->move(x - 2 * shadowWidth, y - shadowWidth);
        controlWnd->resize(wndNormalGeometry.size());
        emit normalized();

        isNormalState = true;
        ui->maximizeButton->setIcon(maximizeIcon);

        // 重绘窗口
        controlWnd->resize(controlWnd->size() + QSize(1, 1));
        controlWnd->resize(controlWnd->size() + QSize(-1, -1));

        // 解锁
        mutex.unlock();
    }
    // 控制窗口双击时最大化或还原
    else if (event->type() == QEvent::MouseButtonDblClick)
    {
        on_maximizeButton_clicked();
    }
    return QWidget::eventFilter(watched, event);
}

void ControlWidget::on_closeButton_clicked()
{
    if (controlWnd == nullptr) return;
    controlWnd->close();
}


void ControlWidget::on_maximizeButton_clicked()
{
    if (controlWnd == nullptr) return;

    if (isNormalState)
    {
        // 获取最大化前的大小
        wndNormalGeometry = controlWnd->geometry();

        // 窗口最大化
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect availableGeometry = screen->availableGeometry();
        controlWnd->setGeometry(availableGeometry);
        emit maximized();

        isNormalState = false;
        ui->maximizeButton->setIcon(normalIcon);

        // 重绘窗口
        controlWnd->resize(controlWnd->size() + QSize(1, 1));
        controlWnd->resize(controlWnd->size() + QSize(-1, -1));
    }
    else
    {
        // 上锁(防止触发事件过滤器中的窗口移动事件)
        mutex.lock();

        // 窗口大小还原
        controlWnd->setGeometry(wndNormalGeometry);
        isNormalState = true;
        ui->maximizeButton->setIcon(maximizeIcon);
        emit normalized();

        // 重绘窗口
        controlWnd->resize(controlWnd->size() + QSize(1, 1));
        controlWnd->resize(controlWnd->size() + QSize(-1, -1));

        // 解锁
        mutex.unlock();
    }
}


void ControlWidget::on_minimizeButton_clicked()
{
    if (controlWnd == nullptr) return;
    controlWnd->showMinimized();
}
