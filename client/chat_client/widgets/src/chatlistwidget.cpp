#include "chatlistwidget.h"
#include "ui_chatlistwidget.h"

#include <QDateTime>
#include <QScrollBar>

ChatListWidget::ChatListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatListWidget)
{
    ui->setupUi(this);

    // 设置搜索框图标
    QAction *searchAction = new QAction(ui->searchLineEdit);
    searchAction->setIcon(QIcon(":/res/img/Search.png"));
    ui->searchLineEdit->addAction(searchAction, QLineEdit::LeadingPosition);

    // 搜索框文本与失焦
    ui->searchLineEdit->setPlaceholderText("搜索");
    connect(ui->searchLineEdit, &QLineEdit::editingFinished, [this]() {
        ui->searchLineEdit->setText("");
    });

    // 设置列表滑动条滚动速度
    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listWidget->verticalScrollBar()->setSingleStep(10);

    // 点击聊天
    connect(ui->listWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item){
        ChatListItem *pItemWidget = item->data(Qt::UserRole).value<ChatListItem *>();
        pItemWidget->hideRedCircle();
        emit selectChat(pItemWidget->getUser());
    });

    connect(this, &ChatListWidget::sig_insertChatItem, this, &ChatListWidget::insertChatItem);
}

ChatListWidget::~ChatListWidget()
{
    delete ui;
    while (ui->listWidget->count())
    {
        QListWidgetItem *item = ui->listWidget->takeItem(0);
        delete item->data(Qt::UserRole).value<ChatListItem *>();
        delete item;
    }
}

int ChatListWidget::removeChatItem(const QString &friendName)
{
    auto iter = items_map.find(friendName);
    if (iter != items_map.end())
    {
        int row = ui->listWidget->row(*iter);
        QListWidgetItem *item = ui->listWidget->takeItem(row);
        ChatListItem *pItemWidget = item->data(Qt::UserRole).value<ChatListItem *>();
        delete pItemWidget;
        delete item;
        items_map.erase(iter);
        return row;
    }
    return -1;
}

ChatListItem *ChatListWidget::getItemWidget(const QString &user)
{
    auto iter = items_map.find(user);
    if (iter != items_map.end())
    {
        return iter.value()->data(Qt::UserRole).value<ChatListItem *>();
    }
    return nullptr;
}

void ChatListWidget::setSelect(const QString &user)
{
    auto iter = items_map.find(user);
    if (iter != items_map.end())
    {
        ui->listWidget->setCurrentItem(*iter);
    }
}

void ChatListWidget::insertChatItem(const QPixmap &profile, const QString &friendName, const QString &msg, const QString &time, int row)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(10, 65));

    ChatListItem *pItemWidget = new ChatListItem(profile, friendName, msg, QDateTime::fromString(time, "yyyy/MM/dd HH:mm").date() == QDate::currentDate() ? time.split(" ")[1] : time.split(" ")[0], 0);
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
    items_map.insert(friendName, item);
}
