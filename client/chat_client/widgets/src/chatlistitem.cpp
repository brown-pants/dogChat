#include "chatlistitem.h"
#include "ui_chatlistitem.h"

ChatListItem::ChatListItem(const QPixmap &profile, const QString &friendName, const QString &msg, const QString &time, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatListItem)
{
    ui->setupUi(this);
    ui->profileLabel->setPixmap(profile);
    ui->friendNameLabel->setText(friendName);
    ui->msgLabel->setText(msg);
    ui->timeLabel->setText(time);
}

ChatListItem::~ChatListItem()
{
    delete ui;
}

void ChatListItem::hideRedCircle()
{
    ui->redCircleWidget->hide();
}

void ChatListItem::showRedCircle()
{
    ui->redCircleWidget->show();
}
