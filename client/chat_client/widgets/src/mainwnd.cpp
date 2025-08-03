#include "mainwnd.h"
#include "ui_mainwnd.h"
#include "tcpclient.h"
#include "storagemanager.h"
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QMovie>
#include <QStyle>
#include <QThread>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>

MainWnd *MainWnd::__instance = nullptr;

MainWnd::MainWnd(QWidget *parent) : QWidget(parent)
    , ui(new Ui::MainWnd)
{
    ui->setupUi(this);

    __instance = this;

    setWindowIcon(QIcon(":/res/icon/anim7.ico"));

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
    m_ChatListWidget = new ChatListWidget(this);
    ui->centerStackedWidget->addWidget(m_ChatListWidget);

    m_FriendListWidget = new FriendListWidget(this);
    ui->centerStackedWidget->addWidget(m_FriendListWidget);

    // 设置右边区域
    m_ChatWidget = new ChatWidget(this);
    ui->rightStackedWidget->addWidget(m_ChatWidget);

    m_NewFriendsWidget = new NewFriendsWidget(this);
    ui->rightStackedWidget->addWidget(m_NewFriendsWidget);

    m_FriendInfoWidget = new FriendInfoWidget(this);
    ui->rightStackedWidget->addWidget(m_FriendInfoWidget);

    // 右侧首页
    QMovie *movie = new QMovie(":/res/gif/anim11.gif");
    movie->start();
    ui->homeLabel->setMovie(movie);
    ui->homeLabel->setFixedSize(movie->frameRect().size());
    p_curChatWidget = ui->homePage;
    p_curFriendWidget = ui->homePage;

    // 初始状态
    ui->centerStackedWidget->setCurrentWidget(m_ChatListWidget);
    ui->rightStackedWidget->setCurrentWidget(ui->homePage);

    // 左侧按钮
    ui->chatButton->setCheckable(true);
    ui->friendButtom->setCheckable(true);
    ui->chatButton->setChecked(true);

    m_userWidget = new FindUserWidget(this);
    connect(m_userWidget, &FindUserWidget::profileChanged, this, [this](const QString &profilePath){
        ui->profileButton->setIcon(QIcon(profilePath));
        TcpClient::GetInstance().ChangeProfile(Util::FileToBase64(profilePath));
        user_profile = QPixmap(profilePath);
    });

    // 最大化时调整边距
    connect(ui->controlWidget, &ControlWidget::maximized, [this]() {
        layout()->setContentsMargins(0, 0, 0, 0);
        setProperty("shadowWidth", -5);
    });

    connect(ui->controlWidget, &ControlWidget::normalized, [this, shadowWidth]() {
        layout()->setContentsMargins(shadowWidth, shadowWidth, shadowWidth, shadowWidth);
        setProperty("shadowWidth", shadowWidth);
    });

    // 新的朋友页面跳转
    connect(m_FriendListWidget, &FriendListWidget::newFriendItemClicked, [this](){
        ui->rightStackedWidget->setCurrentWidget(m_NewFriendsWidget);
        p_curFriendWidget = m_NewFriendsWidget;
    });

    // 选中好友
    connect(m_FriendListWidget, &FriendListWidget::itemClicked, this, [this](const QString &user, const QPixmap &profile){
        m_FriendInfoWidget->setUser(user, profile);
        ui->rightStackedWidget->setCurrentWidget(m_FriendInfoWidget);
        p_curFriendWidget = m_FriendInfoWidget;
    });

    // 显示聊天界面
    connect(m_FriendInfoWidget, &FriendInfoWidget::toChat, this, &MainWnd::toChat);
    connect(m_userWidget, &FindUserWidget::toChat, this, &MainWnd::toChat);
    connect(m_FriendListWidget->m_FindUserWidget, &FindUserWidget::toChat, this, &MainWnd::toChat);

    // 删除好友
    connect(m_FriendInfoWidget, &FriendInfoWidget::removeFriend, this, [this](){
        ui->rightStackedWidget->setCurrentWidget(ui->homePage);
        p_curFriendWidget = ui->homePage;
    });

    // 选择聊天
    connect(m_ChatListWidget, &ChatListWidget::selectChat, this, [this](const QString &user){
        ui->rightStackedWidget->setCurrentWidget(m_ChatWidget);
        p_curChatWidget = m_ChatWidget;
        if (m_ChatWidget->curUser() != user)
        {
            m_ChatWidget->loadMessages(user);
        }
    });

    // 接收到消息
    connect(&TcpClient::GetInstance(), &TcpClient::sig_recvMsg, this, &MainWnd::RecvMsg);

    connect(m_ChatWidget, &ChatWidget::sendMsg, this, [this](const QString &user, ChatMsgInfo msg){
        loadMsg(user, msg);
        m_ChatListWidget->setSelect(user);
    });

    connect(&TcpClient::GetInstance(), &TcpClient::sig_offline, this, [this](){
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("提示");
        msgBox->setText("被顶下线啦");
        connect(msgBox, &QMessageBox::buttonClicked, [=](){
            this->close();
            msgBox->deleteLater(); // 删除消息框对象
        });
        msgBox->show();
    });

    connect(this, &MainWnd::sig_loadMsg, this, [this](const QString &user, ChatMsgInfo msgInfo){
        loadMsg(user, msgInfo);
        // 加载聊天
        if (m_ChatWidget->curUser() == user)
        {
            m_ChatWidget->appendMsg(msgInfo);
            m_ChatListWidget->setSelect(user);
        }
        else
        {
            m_ChatListWidget->getItemWidget(user)->showRedCircle();
        }
    });

    connect(&TcpClient::GetInstance(), &TcpClient::sig_removeFriend_over, this, [this](const QString &user){
        if (ui->rightStackedWidget->currentWidget() == m_FriendInfoWidget && m_FriendInfoWidget->curUser() == user)
        {
            ui->rightStackedWidget->setCurrentWidget(ui->homePage);
            p_curFriendWidget = ui->homePage;
        }
    });

    ui->profileButton->installEventFilter(this);

}

MainWnd *MainWnd::GetInstance()
{
    return __instance;
}

MainWnd::~MainWnd()
{
    delete ui;
}

QString MainWnd::curUser() const
{
    return curr_user;
}

QPixmap MainWnd::curUserProfile() const
{
    return user_profile;
}

QPixmap MainWnd::getUserProfile(const QString &user) const
{
    return user == curr_user ? user_profile : m_FriendListWidget->getFriendProfile(user);
}

bool MainWnd::eventFilter(QObject *obj, QEvent *ev)
{
    // 绘制圆角头像
    if (obj == ui->profileButton && ev->type() == QEvent::Paint)
    {
        QPainter painter(ui->profileButton);
        painter.setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿
        QRect porfileRect = ui->profileButton->rect();
        int radius = 10;

        // 1. 绘制圆形背景
        painter.setOpacity(0.3);
        painter.setBrush(QBrush(qRgb(150, 218, 248)));  // 背景色
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(porfileRect, radius, radius);

        // 2. 创建圆形裁剪路径
        QPainterPath path;
        path.addRoundedRect(0, 0, porfileRect.width(), porfileRect.height(), radius, radius);
        painter.setClipPath(path);  // 设置裁剪区域

        // 3. 绘制图标（超出部分自动被裁剪）
        if (!porfileRect.isNull())
        {
            QPixmap pixmap = ui->profileButton->icon().pixmap(ui->profileButton->iconSize());
            painter.setOpacity(1.0);
            painter.drawPixmap(porfileRect, pixmap, pixmap.rect());
        }
        return true;
    }
    return QWidget::eventFilter(obj, ev);
}

void MainWnd::on_chatButton_clicked()
{
    ui->centerStackedWidget->setCurrentWidget(m_ChatListWidget);
    ui->rightStackedWidget->setCurrentWidget(p_curChatWidget);
    ui->chatButton->setChecked(true);
    ui->friendButtom->setChecked(false);
}


void MainWnd::on_friendButtom_clicked()
{
    ui->centerStackedWidget->setCurrentWidget(m_FriendListWidget);
    ui->rightStackedWidget->setCurrentWidget(p_curFriendWidget);
    ui->friendButtom->setChecked(true);
    ui->chatButton->setChecked(false);
}

void MainWnd::on_profileButton_clicked()
{
    m_userWidget->setInfo(curr_user, user_profile, true);
    m_userWidget->move(QCursor::pos() - QPoint(20, 20));
    m_userWidget->show();
}

void MainWnd::RecvMsg(const QString &user, const QString &time, const QString &msg, bool file_msg)
{
    ChatMsgInfo msgInfo(time, "text", msg, user, false);

    if (file_msg)
    {
        msgInfo.type = "file";
        if (msg.contains(" "))
        {// 包含空格
            QStringList list = msg.split(" ");
            QString fileName = list[0];
            QString base64 = list[1];
            // 创建文件

            // base64转码需要很长时间 创建子线程完成
            QFutureWatcher<ChatMsgInfo> *watcher = new QFutureWatcher<ChatMsgInfo>(this);
            connect(watcher, &QFutureWatcher<ChatMsgInfo>::finished, this, [watcher, time, user, this]() {
                ChatMsgInfo msgInfo = watcher->result();
                emit sig_loadMsg(user, msgInfo); // 触发信号
                watcher->deleteLater();
            });
            QFuture<ChatMsgInfo> future = QtConcurrent::run([=]() {
                ChatMsgInfo info = msgInfo;
                info.msg = StorageManager::GetInstance().saveFile(curr_user, user, fileName, QByteArray::fromBase64(base64.toUtf8()));
                return info;
            });
            watcher->setFuture(future);
            return;
        }
        else
        {// 内置表情
            msgInfo.msg = msg;
        }
    }

    loadMsg(user, msgInfo);

    // 加载聊天
    if (m_ChatWidget->curUser() == user)
    {
        m_ChatWidget->appendMsg(msgInfo);
        m_ChatListWidget->setSelect(user);
    }
    else
    {
        m_ChatListWidget->getItemWidget(user)->showRedCircle();
    }
}

void MainWnd::loadMsg(const QString &user, ChatMsgInfo msg)
{
    int row = m_ChatListWidget->removeChatItem(user);
    m_ChatListWidget->insertChatItem(getUserProfile(user), user, msg.type == "text" ? msg.msg : "[文件]", msg.time, 0);

    // 多线程
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);

    connect(watcher, &QFutureWatcher<void>::finished, this, [watcher]() {
        qDebug() << "loadMsg  end" << Qt::endl;
        watcher->deleteLater();
    });

    QFuture<void> future = QtConcurrent::run([=]() {
        StorageManager::GetInstance().loadChatMsg(curr_user, user);
        StorageManager::GetInstance().addChatMsg(user, msg);
        StorageManager::GetInstance().saveChatMsg(curr_user, user);

        if (row >= 0)
        {
            StorageManager::GetInstance().removeFromChatList(row);
        }
        StorageManager::GetInstance().insertToChatList(user, 0);
        StorageManager::GetInstance().saveChatList(curr_user);
    });

    watcher->setFuture(future);
}

void MainWnd::toChat(const QString &user)
{
    int row = m_ChatListWidget->removeChatItem(user);
    const QPixmap &profile = getUserProfile(user);

    // 加载消息
    StorageManager::GetInstance().loadChatMsg(curr_user, user);
    if (StorageManager::GetInstance().chatMsgCount(user) > 0)
    {
        ChatMsgInfo msg = StorageManager::GetInstance().getChatMsg(user, 0);
        m_ChatListWidget->insertChatItem(profile, user, msg.type == "text" ? msg.msg : "[文件]", msg.time, 0);
    }
    else
    {
        m_ChatListWidget->insertChatItem(profile, user, "", "", 0);
    }

    // 加载聊天
    if (m_ChatWidget->curUser() != user)
    {
        m_ChatWidget->loadMessages(user);
    }

    on_chatButton_clicked();
    ui->rightStackedWidget->setCurrentWidget(m_ChatWidget);
    p_curChatWidget = m_ChatWidget;
    if (row >= 0)
    {
        StorageManager::GetInstance().removeFromChatList(row);
    }
    StorageManager::GetInstance().insertToChatList(user, 0);
    StorageManager::GetInstance().saveChatList(curr_user);

    m_ChatListWidget->setSelect(user);
}

MainWndLoader::MainWndLoader(QObject *parent) : QObject(parent)
{
    mainwnd = MainWnd::__instance;
}

MainWndLoader::~MainWndLoader()
{
    qDebug() << "~MainWndLoader()" << Qt::endl;
}

void MainWndLoader::load(const QString &user, const QPixmap &profile)
{
    MainWnd *mainwnd = MainWnd::__instance;
    mainwnd->curr_user = user;
    mainwnd->user_profile = profile;
    mainwnd->ui->profileButton->setIconSize(mainwnd->ui->profileButton->size());
    mainwnd->ui->profileButton->setIcon(QIcon(mainwnd->user_profile));

    // 好友列表加载槽连接
    connect(&TcpClient::GetInstance(), &TcpClient::sig_recvFriends_over, this, &MainWndLoader::load_Friends);

    // 好友申请列表加载槽连接
    connect(&TcpClient::GetInstance(), &TcpClient::sig_recvApplyFriendRequest_over, this, &MainWndLoader::load_applyFriends);

    // 好友列表加载
    TcpClient::GetInstance().RecvFriends(user);
}

void MainWndLoader::load_Friends(const QVector<FriendInfo> &friend_arr)
{
    qDebug() << "load friend list" << Qt::endl;

    std::set<FriendInfo> friends;
    for (const FriendInfo &fri : friend_arr)
    {
        friends.insert(FriendInfo(fri.name, fri.profile));
    }
    mainwnd->m_FriendListWidget->loadFriends(friends);
    // 好友申请列表加载
    TcpClient::GetInstance().RecvApplyFriendRequest(mainwnd->curr_user);
}

void MainWndLoader::load_applyFriends(const QVector<ApplyFriendInfo> &apply_arr)
{
    qDebug() << "load apply friend request list" << Qt::endl;
    for (auto r_it = apply_arr.rbegin(); r_it != apply_arr.rend(); r_it ++)
    {
        QPixmap profile;
        profile.loadFromData(QByteArray::fromBase64(r_it->profile.toUtf8()));
        emit mainwnd->m_NewFriendsWidget->sig_insertFriendItem(profile, r_it->user_apply, r_it->leave_msg, r_it->apply_state);
        QThread::msleep(10);
    }

    // 加载聊天列表
    load_chatList();

    // 加载等待消息
    TcpClient::GetInstance().LoadWaitMsg();

    emit finished();
}

void MainWndLoader::load_chatList()
{
    StorageManager::GetInstance().loadChatList(mainwnd->curUser());
    const QVector<QString> &chatList = StorageManager::GetInstance().chatList();
    for (const QString &user : chatList)
    {
        const QPixmap &profile = (user == mainwnd->curUser() ? mainwnd->user_profile : mainwnd->m_FriendListWidget->getFriendProfile(user));
        if (!profile.isNull())
        {
            StorageManager::GetInstance().loadChatMsg(mainwnd->curUser(), user);
            if (StorageManager::GetInstance().chatMsgCount(user) == 0)
            {
                emit mainwnd->m_ChatListWidget->sig_insertChatItem(profile, user, "", "");
            }
            else
            {
                ChatMsgInfo msg = StorageManager::GetInstance().getChatMsg(user, 0);
                emit mainwnd->m_ChatListWidget->sig_insertChatItem(profile, user, msg.type == "text" ? msg.msg : "[文件]", msg.time);
            }
        }
    }
}
