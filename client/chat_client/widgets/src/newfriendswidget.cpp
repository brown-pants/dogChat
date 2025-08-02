#include "newfriendswidget.h"
#include "ui_newfriendswidget.h"
#include "newfriendsitem.h"
#include "tcpclient.h"

NewFriendsWidget::NewFriendsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NewFriendsWidget)
{
    ui->setupUi(this);
    connect(this, &NewFriendsWidget::sig_insertFriendItem, this, &NewFriendsWidget::insertFriendItem);
    connect(&TcpClient::GetInstance(), &TcpClient::sig_applyFriendRequest, this, [this](const QString &user, const QString &leave, const QString &profile){
        qDebug() << "new friend apply: " << user << Qt::endl;
        QPixmap profilePixmap;
        profilePixmap.loadFromData(QByteArray::fromBase64(profile.toUtf8()));
        insertFriendItem(profilePixmap, user, leave, "wait", 0);
    });
}

NewFriendsWidget::~NewFriendsWidget()
{
    delete ui;
    while (ui->listWidget->count())
    {
        QListWidgetItem *item = ui->listWidget->takeItem(0);
        delete item->data(Qt::UserRole).value<NewFriendsItem *>();
        delete item;
    }
}

void NewFriendsWidget::insertFriendItem(const QPixmap &profile, const QString &friendName, const QString &leave_msg, const QString &apply_state, int row)
{
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(10, 65));

    NewFriendsItem *pItemWidget = new NewFriendsItem(profile, friendName, leave_msg, apply_state, item, ui->listWidget);
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
}
