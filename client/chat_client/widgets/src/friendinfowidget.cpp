#include "friendinfowidget.h"
#include "ui_friendinfowidget.h"
#include "tcpclient.h"
#include <QMenu>

FriendInfoWidget::FriendInfoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendInfoWidget)
{
    ui->setupUi(this);
}

QString FriendInfoWidget::curUser() const
{
    return user;
}

FriendInfoWidget::~FriendInfoWidget()
{
    delete ui;
}

void FriendInfoWidget::setUser(const QString &user, const QPixmap &profile)
{
    this->user = user;
    ui->nameLabel->setText(user);
    ui->profileLabel->setPixmap(profile);
}

void FriendInfoWidget::on_settingButton_clicked()
{
    QMenu menu;
    QAction *del_action = menu.addAction("删除好友");
    if (menu.exec(QCursor::pos()) == del_action)
    {
        TcpClient::GetInstance().RemoveFriend(user);
        emit removeFriend();
    }
}


void FriendInfoWidget::on_chatButton_clicked()
{
    emit toChat(user);
}

