#include "friendlistitem.h"
#include "ui_friendlistitem.h"

FriendListItem::FriendListItem(const QPixmap &profile, const QString &friendName, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendListItem)
{
    ui->setupUi(this);
    ui->profileLabel->setPixmap(profile);
    ui->profileLabel->setScaledContents(true);
    ui->nameLabel->setText(friendName);
}

FriendListItem::~FriendListItem()
{
    delete ui;
}
