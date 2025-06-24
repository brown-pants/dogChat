#include "friendlistwidget.h"
#include "ui_friendlistwidget.h"
#include "friendlistitem.h"

#include <QScrollBar>

FriendListWidget::FriendListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendListWidget)
{
    ui->setupUi(this);

    // 设置搜索框图标
    QAction *searchAction = new QAction(ui->searchLineEdit);
    searchAction->setIcon(QIcon(":/res/img/Search.png"));
    ui->searchLineEdit->addAction(searchAction, QLineEdit::LeadingPosition);

    // 设置列表滑动条滚动速度
    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listWidget->verticalScrollBar()->setSingleStep(10);



    std::multiset<FriendInfo> friends;
    friends.insert(FriendInfo("", "小鸡毛", QPixmap("D:\\code\\MyChat\\xjm.gif")));
    friends.insert(FriendInfo("", "小白", QPixmap("D:\\code\\MyChat\\xjm.gif")));
    friends.insert(FriendInfo("", "大鸡毛", QPixmap("D:\\code\\MyChat\\xjm.gif")));
    friends.insert(FriendInfo("", "嘟嘟", QPixmap("D:\\code\\MyChat\\xjm.gif")));
    friends.insert(FriendInfo("", "噜噜", QPixmap("D:\\code\\MyChat\\xjm.gif")));
    friends.insert(FriendInfo("", "miya", QPixmap("D:\\code\\MyChat\\xjm.gif")));
    friends.insert(FriendInfo("", "yucheng", QPixmap("D:\\code\\MyChat\\xjm.gif")));
    friends.insert(FriendInfo("", "A", QPixmap("D:\\code\\MyChat\\xjm.gif")));
    loadFriends(friends);
}

FriendListWidget::~FriendListWidget()
{
    delete ui;
}

void FriendListWidget::insertLabelItem(const QString &text, int row)
{
    if (row < 0)
    {
        ui->listWidget->addItem(QString(text));
    }
    else
    {
        ui->listWidget->insertItem(row, text);
    }
    QListWidgetItem *itemLabel = ui->listWidget->item(ui->listWidget->count() - 1);
    itemLabel->setFlags(itemLabel->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);

}

void FriendListWidget::insertFriendItem(const QPixmap &profile, const QString &friendName, int row)
{
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(QSize(10, 65));
    FriendListItem *pItemWidget = new FriendListItem(profile, friendName, ui->listWidget);
    item->setData(Qt::UserRole, QVariant::fromValue(pItemWidget));
    ui->listWidget->setItemWidget(item, pItemWidget);
    if (row < 0)
    {
        ui->listWidget->addItem(item);
    }
    else
    {
        ui->listWidget->insertItem(row, item);
    }
}

void FriendListWidget::loadFriends(const std::multiset<FriendInfo> &friends)
{
    char preInitail = '\0';
    for (const FriendInfo &info : friends)
    {
        char nameInitail = Util::GetInitial(info.name);
        if (preInitail == nameInitail)
        {
            insertFriendItem(info.profile, info.name);
        }
        else
        {
            insertLabelItem(QString(Util::GetInitial(info.name)));
            insertFriendItem(info.profile, info.name);
        }
        preInitail = nameInitail;
    }
}
