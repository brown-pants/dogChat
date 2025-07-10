#include "mainwnd.h"
#include "ui_mainwnd.h"
#include "chatlistwidget.h"
#include "chatwidget.h"
#include "friendlistwidget.h"

#include <QGraphicsDropShadowEffect>
#include <QMovie>

MainWnd::MainWnd(const QString &user, const QPixmap &profile, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWnd), curr_user(user), user_profile(profile)
{
    ui->setupUi(this);

    // 窗口无控制栏且透明
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 窗口可拖拽伸缩
    setProperty("canMove", true);
    setProperty("canResize", true);

    // 设置外边框阴影参数
    int shadowWidth = 20;
    setProperty("shadowWidth", shadowWidth);
    layout()->setContentsMargins(shadowWidth, shadowWidth, shadowWidth, shadowWidth);

    // 设置外边框阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(50, 50, 50));
    shadow->setBlurRadius(shadowWidth);
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
    ui->rightStackedWidget->setCurrentWidget(ui->homePage);

    // 左侧按钮
    ui->chatButton->setCheckable(true);
    ui->friendButtom->setCheckable(true);
    ui->chatButton->setChecked(true);

    // 最大化时调整边距
    connect(ui->controlWidget, &ControlWidget::maximized, [this]() {
        layout()->setContentsMargins(0, 0, 0, 0);
        setProperty("shadowWidth", -5);
    });

    connect(ui->controlWidget, &ControlWidget::normalized, [this, shadowWidth]() {
        layout()->setContentsMargins(shadowWidth, shadowWidth, shadowWidth, shadowWidth);
        setProperty("shadowWidth", shadowWidth);
    });

    ui->profileButton->setIcon(QIcon(user_profile));
}

MainWnd &MainWnd::GetInstance(const QString &user, const QPixmap &profile)
{
    static MainWnd instance(user, profile);
    return instance;
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

