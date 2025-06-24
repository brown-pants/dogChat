#include "mainwnd.h"
#include "ui_mainwnd.h"
#include "chatlistwidget.h"
#include "chatWidget.h"
#include "friendlistwidget.h"

#include <QGraphicsDropShadowEffect>
#include <QMovie>

MainWnd::MainWnd(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWnd)
{
    ui->setupUi(this);

    // 窗口无控制栏且透明
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 窗口可拖拽伸缩
    setProperty("canMove", true);
    setProperty("canResize", true);

    // 设置外边框阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(50, 50, 50));
    shadow->setBlurRadius(WINDOW_SHADOW_WIDTH);
    ui->shadowWidget->setGraphicsEffect(shadow);

    // 设置分离器鼠标样式
    ui->splitter->handle(1)->setCursor(Qt::SizeHorCursor);

    // 窗口伸缩左边宽度不会改变
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);

    // 设置控制栏
    ui->controlWidget->setControlWnd(this);

    // 设置中间区域
    ChatListWidget *chatListWidget = new ChatListWidget(this);
    ui->centerStackedWidget->insertWidget(0, chatListWidget);

    FriendListWidget *friendListWidget = new FriendListWidget(this);
    ui->centerStackedWidget->insertWidget(1, friendListWidget);

    // 设置右边区域
    ChatWidget *chatWidget = new ChatWidget(this);
    ui->rightStackedWidget->insertWidget(0, chatWidget);

    // 右侧首页
    QMovie *movie = new QMovie(":/res/gif/anim11.gif");
    movie->start();
    ui->homeLabel->setMovie(movie);
    ui->homeLabel->setFixedSize(movie->frameRect().size());

    // 初始状态
    ui->centerStackedWidget->setCurrentIndex(0);
    ui->rightStackedWidget->setCurrentIndex(0);

    // 左侧按钮
    ui->chatButton->setCheckable(true);
    ui->friendButtom->setCheckable(true);
    ui->chatButton->setChecked(true);
}

MainWnd::~MainWnd()
{
    delete ui;
}

void MainWnd::on_chatButton_clicked()
{
    ui->centerStackedWidget->setCurrentIndex(0);
    ui->chatButton->setChecked(true);
    ui->friendButtom->setChecked(false);
}


void MainWnd::on_friendButtom_clicked()
{
    ui->centerStackedWidget->setCurrentIndex(1);
    ui->friendButtom->setChecked(true);
    ui->chatButton->setChecked(false);
}

