#include "chatlistitem.h"
#include "ui_chatlistitem.h"

ChatListItem::ChatListItem(const QPixmap &profile, const QString &friendName, const QString &msg, const QString &time, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatListItem), msgText(msg)
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

void ChatListItem::resizeEvent(QResizeEvent *e)
{
    // 超出文本范围显示...
    QFontMetrics metrics(ui->msgLabel->font());
    QString elidedText = metrics.elidedText(msgText, Qt::ElideRight, ui->msgLabel->width());
    ui->msgLabel->setText(elidedText);
    if(elidedText != msgText)
    {
        ui->msgLabel->setToolTip(msgText);
    }
    else
    {
        ui->msgLabel->setToolTip("");
    }
}
