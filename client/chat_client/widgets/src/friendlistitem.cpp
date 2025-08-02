#include "friendlistitem.h"
#include "ui_friendlistitem.h"

FriendListItem::FriendListItem(const QPixmap &profile, const QString &friendName, bool find, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendListItem), friendNameText(friendName), profile(profile), m_find(find)
{
    ui->setupUi(this);
    ui->profileLabel->setPixmap(profile);
    ui->profileLabel->setScaledContents(true);
    ui->nameLabel->setTextFormat(Qt::RichText);
    ui->nameLabel->setText(friendName);
}

FriendListItem::~FriendListItem()
{
    delete ui;
}

void FriendListItem::setText(const QString &text)
{
    if (m_find)
    {
        QString plainText = QString("搜索：<font color='green'>%1</font>").arg(text);
        ui->nameLabel->setText(plainText);
        friendNameText = plainText;
    }
    else
    {
        ui->nameLabel->setText(text);
        friendNameText = text;
    }
    resizeEvent(nullptr);
}

QString FriendListItem::getUserName()
{
    return friendNameText;
}

QPixmap FriendListItem::getUserProfile()
{
    return profile;
}

void FriendListItem::resizeEvent(QResizeEvent *e)
{
    // 超出文本范围显示...
    QTextDocument doc;
    doc.setHtml(friendNameText);
    QString plainText = doc.toPlainText();

    QFontMetrics metrics(ui->nameLabel->font());

    QString elidedPlain = metrics.elidedText(plainText, Qt::ElideRight, ui->nameLabel->width());
    if(m_find)
    {
        elidedPlain = elidedPlain.mid(QString("搜索：").length());
        ui->nameLabel->setText(QString("搜索：<font color='green'>%1</font>").arg(elidedPlain));
        elidedPlain = "搜索：" + elidedPlain;
    }
    else
    {
        ui->nameLabel->setText(elidedPlain);
    }

    if (elidedPlain != plainText)
    {
        ui->nameLabel->setToolTip(friendNameText);
    }
    else
    {
        ui->nameLabel->setToolTip("");
    }
}
