#include "newfriendsitem.h"
#include "ui_newfriendsitem.h"
#include "TcpClient.h"
#include <QMouseEvent>
#include <QMenu>

NewFriendsItem::NewFriendsItem(const QPixmap &profile, const QString &friendName, const QString &leave_msg, const QString &apply_state, QListWidgetItem *item, QListWidget *listWidget)
    : QWidget(nullptr)
    , ui(new Ui::NewFriendsItem), name(friendName), leave(leave_msg), m_item(item), m_listWidget(listWidget)
{
    ui->setupUi(this);
    ui->profileLabel->setPixmap(profile);
    ui->nameLabel->setText(friendName);
    ui->leaveLabel->setText(leave_msg);
    if (apply_state == "wait")
    {
        ui->stateLabel->hide();
    }
    else if (apply_state == "pass")
    {
        ui->stateLabel->setText("已添加");
        ui->passButton->hide();
        ui->refuseButton->hide();
    }
    else
    {
        ui->stateLabel->setText("已拒绝");
        ui->passButton->hide();
        ui->refuseButton->hide();
    }
}

NewFriendsItem::~NewFriendsItem()
{
    delete ui;
}

void NewFriendsItem::resizeEvent(QResizeEvent *e)
{
    // 超出文本范围显示...
    QFontMetrics metrics(ui->nameLabel->font());
    QString elidedText = metrics.elidedText(name, Qt::ElideRight, ui->nameLabel->width());
    ui->nameLabel->setText(elidedText);
    if(elidedText != name)
    {
        ui->nameLabel->setToolTip(name);
    }
    else
    {
        ui->nameLabel->setToolTip("");
    }

    // 多行数据
    int idx_r = leave.indexOf("\n");
    if (idx_r != -1)
    {
        const QString line = leave.mid(0, idx_r);
        elidedText = metrics.elidedText(line, Qt::ElideRight, ui->leaveLabel->width());
        if (elidedText == line)
        {
            elidedText += "...";
        }
        ui->leaveLabel->setText(elidedText);
        ui->leaveLabel->setToolTip(leave);
        return;
    }

    // 单行数据
    metrics = QFontMetrics(ui->leaveLabel->font());
    elidedText = metrics.elidedText(leave, Qt::ElideRight, ui->leaveLabel->width());
    ui->leaveLabel->setText(elidedText);
    if(elidedText != leave)
    {
        ui->leaveLabel->setToolTip(leave);
    }
    else
    {
        ui->leaveLabel->setToolTip("");
    }
}

void NewFriendsItem::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
    {
        QMenu menu;
        QAction *del_action = menu.addAction("删除");
        if (menu.exec(e->globalPosition().toPoint()) == del_action)
        {
            m_listWidget->removeItemWidget(m_item);
            delete m_item;
            deleteLater();
            TcpClient::GetInstance().RemoveFriendApply(name);
        }
    }
}

void NewFriendsItem::on_passButton_clicked()
{
    ui->stateLabel->setText("已添加");
    ui->stateLabel->show();
    ui->passButton->hide();
    ui->refuseButton->hide();
    TcpClient::GetInstance().PassFriendApply(name);
}


void NewFriendsItem::on_refuseButton_clicked()
{
    ui->stateLabel->setText("已拒绝");
    ui->stateLabel->show();
    ui->passButton->hide();
    ui->refuseButton->hide();
    TcpClient::GetInstance().RefuseFriendApply(name);
}

