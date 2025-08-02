#include "friendlistwidget.h"
#include "ui_friendlistwidget.h"
#include "friendlistitem.h"
#include "TcpClient.h"
#include <QScrollBar>
#include <QMessageBox>
#include <QThread>

FriendListWidget::FriendListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendListWidget), m_state(0)
{
    ui->setupUi(this);

    m_FindUserWidget = new FindUserWidget(this);

    // 设置搜索框图标
    QAction *searchAction = new QAction(ui->searchLineEdit);
    searchAction->setIcon(QIcon(":/res/img/Search.png"));
    ui->searchLineEdit->addAction(searchAction, QLineEdit::LeadingPosition);

    QAction *findAction = new QAction(ui->findFriendEdit);
    findAction->setIcon(QIcon(":/res/img/User_black.png"));
    ui->findFriendEdit->addAction(findAction, QLineEdit::LeadingPosition);

    // 好友查询
    insertLabelItem("无法找到该用户，请检查用户名是否正确");
    insertFriendItem(QPixmap(":/res/img/find.png"), "搜索：", true);
    ui->listWidget->item(0)->setHidden(true);
    ui->listWidget->item(1)->setHidden(true);

    // 好友申请
    insertLabelItem(QString("新的朋友"));
    insertFriendItem(QPixmap(":/res/img/friend.jpeg"), "新的朋友");

    // 搜索框文本
    ui->searchLineEdit->setPlaceholderText("搜索");
    ui->findFriendEdit->setPlaceholderText("用户名");

    // 搜索框文本改变
    connect(ui->searchLineEdit, &QLineEdit::textChanged, [this](const QString &text){

    });

    connect(ui->findFriendEdit, &QLineEdit::textChanged, [this](const QString &text){
        QListWidgetItem *item = ui->listWidget->item(1);
        FriendListItem *w = item->data(Qt::UserRole).value<FriendListItem *>();
        ui->listWidget->item(0)->setHidden(true);
        if (text == "")
        {
            item->setHidden(true);
        }
        else
        {
            w->setText(text);
            item->setHidden(false);
        }
    });

    connect(ui->listWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item){
        int row = ui->listWidget->row(item);
        if (row == 1)
        {// 搜索
            if (m_mtx.tryLock())
            {// 线程锁防止连续点击
                const QString &user = ui->findFriendEdit->text();
                TcpClient::GetInstance().FindUser(user);
            }
        }
        else if (row == 3)
        {
            emit newFriendItemClicked();
        }
        else
        {
            FriendListItem *pItemWidget = item->data(Qt::UserRole).value<FriendListItem *>();
            if (pItemWidget)
            {
                emit itemClicked(pItemWidget->getUserName(), pItemWidget->getUserProfile());
            }
        }
    });

    connect(&TcpClient::GetInstance(), &TcpClient::sig_findUser_over, this, &FriendListWidget::slo_findFriend);
    connect(&TcpClient::GetInstance(), &TcpClient::sig_addedFriend_over, this, &FriendListWidget::addFriend);
    connect(&TcpClient::GetInstance(), &TcpClient::sig_removeFriend_over, this, &FriendListWidget::removeFriend);
    connect(this, &FriendListWidget::sig_insertLabelItem, this, &FriendListWidget::insertLabelItem);
    connect(this, &FriendListWidget::sig_insertFriendItem, this, &FriendListWidget::insertFriendItem);

    // 设置列表滑动条滚动速度
    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listWidget->verticalScrollBar()->setSingleStep(10);

    ui->cancelButton->hide();
    ui->findFriendEdit->hide();


}

FriendListWidget::~FriendListWidget()
{
    delete ui;
    while (ui->listWidget->count() != 0)
    {
        QListWidgetItem *item = ui->listWidget->takeItem(0);
        delete item->data(Qt::UserRole).value<FriendListItem *>();
        delete item;
    }
}

void FriendListWidget::insertLabelItem(const QString &text, int row)
{
    QListWidgetItem *itemLabel = nullptr;
    if (row < 0)
    {
        ui->listWidget->addItem(QString(text));
        itemLabel = ui->listWidget->item(ui->listWidget->count() - 1);
    }
    else
    {
        ui->listWidget->insertItem(row, text);
        itemLabel = ui->listWidget->item(row);
    }
    itemLabel->setFlags(itemLabel->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
    itemLabel->setForeground(QBrush(Qt::gray));

    if (m_state == 1)
    {
        itemLabel->setHidden(true);
    }
}

void FriendListWidget::insertFriendItem(const QPixmap &profile, const QString &friendName, bool find, int row)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(10, 65));

    FriendListItem *pItemWidget = new FriendListItem(profile, friendName, find);
    item->setData(Qt::UserRole, QVariant::fromValue(pItemWidget));

    if (row < 0)
    {
        ui->listWidget->addItem(item);
    }
    else
    {
        ui->listWidget->insertItem(row, item);
    }
    ui->listWidget->setItemWidget(item, pItemWidget);

    if (m_state == 1)
    {
        item->setHidden(true);
    }

    profiles.insert(friendName, profile);
}

void FriendListWidget::loadFriends(const std::set<FriendInfo> &friends)
{
    char preInitail = '\0';
    for (const FriendInfo &info : friends)
    {
        char nameInitail = Util::GetInitial(info.name);
        if (preInitail == nameInitail)
        {
            emit sig_insertFriendItem(info.profile, info.name);
        }
        else
        {
            // 获取首字母
            const QString &initail = QString(Util::GetInitial(info.name));
            emit sig_insertLabelItem(initail);
            emit sig_insertFriendItem(info.profile, info.name);
        }
        preInitail = nameInitail;
        QThread::msleep(10);
    }
}

QPixmap FriendListWidget::getFriendProfile(const QString &user) const
{
    auto iter = profiles.find(user);
    if (iter == profiles.end())
    {
        return QPixmap();
    }
    return *iter;
}

void FriendListWidget::on_addFriendButton_clicked()
{
    m_state = 1;
    ui->findFriendEdit->clear();
    ui->cancelButton->show();
    ui->findFriendEdit->show();
    ui->addFriendButton->hide();
    ui->searchLineEdit->hide();
    for (int i = 0; i < ui->listWidget->count(); i ++)
    {
        ui->listWidget->item(i)->setHidden(true);
    }
}


void FriendListWidget::on_cancelButton_clicked()
{
    m_state = 0;
    ui->listWidget->item(0)->setHidden(true);
    ui->searchLineEdit->clear();
    ui->cancelButton->hide();
    ui->findFriendEdit->hide();
    ui->addFriendButton->show();
    ui->searchLineEdit->show();
    for (int i = 0; i < ui->listWidget->count(); i ++)
    {
        if (i < 2)  ui->listWidget->item(i)->setHidden(true);
        else        ui->listWidget->item(i)->setHidden(false);
    }
}

void FriendListWidget::slo_findFriend(const QString &status, const QString &profile, const QString &isFriend)
{
    if (status == "success")
    {
        QPixmap profilePixmap;
        QByteArray profileData = QByteArray::fromBase64(profile.toUtf8());
        profilePixmap.loadFromData(profileData);
        m_FindUserWidget->setInfo(ui->findFriendEdit->text(), profilePixmap, isFriend == "1" ? true : false);
        m_FindUserWidget->move(ui->searchWidget->mapToGlobal(ui->searchWidget->pos()));
        m_FindUserWidget->show();
    }
    else if (status == "fail")
    {
        ui->listWidget->item(0)->setHidden(false);
        ui->listWidget->item(1)->setHidden(true);
    }
    else
    {
        QMessageBox::information(this, "消息", "服务器异常，查找失败");
    }
    m_mtx.unlock(); // 解锁点击可以查询
}

void FriendListWidget::addFriend(const QString &user, const QString &profile)
{
    qDebug() << "add friend: " << user << Qt::endl;

    QPixmap profilePixmap;
    profilePixmap.loadFromData(QByteArray::fromBase64(profile.toUtf8()));

    // 获取首字母
    const QString &initail = QString(Util::GetInitial(user));
    const QString &pinyin = Util::Pinyin(user);
    int itemCount = ui->listWidget->count();
    bool hasInitail = false;
    int row;

    // 查找首字母
    for (row = 4; row < itemCount; row ++)
    {
        QListWidgetItem *item = ui->listWidget->item(row);
        FriendListItem *friendItem = item->data(Qt::UserRole).value<FriendListItem *>();

        if (friendItem == nullptr && !hasInitail)
        {// 首字母标签
            const QString &initailText = item->text();
            if (initailText == initail)
            {// 有这个首字母存在
                hasInitail = true;
            }
        }
        else if ((friendItem == nullptr && hasInitail) || (friendItem && pinyin < Util::Pinyin(friendItem->getUserName())))
        {
            break;
        }
    }

    // 当前列表中没有这个首字母
    if (!hasInitail)
    {
        qDebug() << "无首字母 " << Qt::endl;
        // 在最末尾
        if (row >= itemCount)
        {
            qDebug() << "在最末尾 " << Qt::endl;
            insertLabelItem(initail);
            insertFriendItem(profilePixmap, user);
        }
        // row一定在首字母后的第一项
        else
        {
            qDebug() << "不在末尾 " << Qt::endl;
            insertLabelItem(initail, row - 1);
            insertFriendItem(profilePixmap, user, false, row);
        }
    }
    else
    {
        qDebug() << "有首字母 " << Qt::endl;
        // 在最末尾
        if (row >= itemCount)
        {
            qDebug() << "在最末尾 " << Qt::endl;
            insertFriendItem(profilePixmap, user);
        }
        else
        {
            qDebug() << "不在末尾 " << Qt::endl;
            insertFriendItem(profilePixmap, user, false, row);
        }
    }
}

void FriendListWidget::removeFriend(const QString &user)
{
    int row;
    int itemCount = ui->listWidget->count();
    QListWidgetItem *labelLWItem = nullptr;
    QListWidgetItem *friendLWItem = nullptr;
    const QString &initail = QString(Util::GetInitial(user));

    for (row = 4; row < itemCount; row ++)
    {
        QListWidgetItem *item = ui->listWidget->item(row);
        FriendListItem *friendItem = item->data(Qt::UserRole).value<FriendListItem *>();

        if (friendItem == nullptr)
        {// 首字母标签
            const QString &initailText = item->text();
            if (initailText == initail)
            {// 有这个首字母存在
                labelLWItem = item;
            }
        }
        else if (friendItem->getUserName() == user)
        {
            friendLWItem = item;
            break;
        }
    }
    if (labelLWItem && ui->listWidget->row(labelLWItem) + 1 == ui->listWidget->row(friendLWItem))
    {
        delete ui->listWidget->takeItem(ui->listWidget->row(labelLWItem));
    }
    delete friendLWItem->data(Qt::UserRole).value<FriendListItem *>();
    delete ui->listWidget->takeItem(ui->listWidget->row(friendLWItem));
}
