#include "chatlistitem.h"
#include "ui_chatlistitem.h"

ChatListItem::ChatListItem(const QPixmap &profile, const QString &friendName, const QString &msg, const QString &time, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatListItem), msgText(msg), user(friendName)
{
    ui->setupUi(this);
    ui->profileLabel->setPixmap(profile);
    ui->friendNameLabel->setText(friendName);
    ui->msgLabel->setText(msg);
    ui->timeLabel->setText(time);
    ui->profileLabel->installEventFilter(this);
    hideRedCircle();
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

QString ChatListItem::getUser() const
{
    return user;
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

    // 多行数据
    int idx_r = msgText.indexOf("\n");
    if (idx_r != -1)
    {
        const QString line = msgText.mid(0, idx_r);
        elidedText = metrics.elidedText(line, Qt::ElideRight, ui->msgLabel->width());
        if (elidedText == line)
        {
            elidedText += "...";
        }
        ui->msgLabel->setText(elidedText);
        ui->msgLabel->setToolTip(msgText);
        return;
    }

    metrics = QFontMetrics(ui->friendNameLabel->font());
    elidedText = metrics.elidedText(user, Qt::ElideRight, ui->friendNameLabel->width());
    ui->friendNameLabel->setText(elidedText);
    if(elidedText != user)
    {
        ui->friendNameLabel->setToolTip(user);
    }
    else
    {
        ui->friendNameLabel->setToolTip("");
    }
}

