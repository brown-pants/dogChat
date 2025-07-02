#include "friendlistitem.h"
#include "ui_friendlistitem.h"

FriendListItem::FriendListItem(const QPixmap &profile, const QString &friendName, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendListItem), friendNameText(friendName)
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

void FriendListItem::resizeEvent(QResizeEvent *e)
{
    // 超出文本范围显示...
    QFontMetrics metrics(ui->nameLabel->font());
    QString elidedText = metrics.elidedText(friendNameText, Qt::ElideRight, ui->nameLabel->width());
    ui->nameLabel->setText(elidedText);
    if(elidedText != friendNameText)
    {
        ui->nameLabel->setToolTip(friendNameText);
    }
    else
    {
        ui->nameLabel->setToolTip("");
    }
}
