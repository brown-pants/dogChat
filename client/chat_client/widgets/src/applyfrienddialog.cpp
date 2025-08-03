#include "applyfrienddialog.h"
#include "tcpclient.h"
#include "ui_applyfrienddialog.h"
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>

ApplyFriendDialog::ApplyFriendDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ApplyFriendDialog)
{
    ui->setupUi(this);

    // 窗口无控制栏透明
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);

    setFixedSize(350, 400);

    // 设置外边框阴影参数
    int shadowWidth = 20;
    layout()->setContentsMargins(shadowWidth, shadowWidth, shadowWidth, shadowWidth);

    //设置外边框阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(120, 120, 120));
    shadow->setBlurRadius(shadowWidth);
    ui->shadowWidget->setGraphicsEffect(shadow);

    // 窗口可移动
    setProperty("canMove", true);

    connect(&TcpClient::GetInstance(), &TcpClient::sig_applyFriend_over, this, [this](const QString &status){
        QString text;
        if (status == "success")
        {
            text = "好友申请已发送";
        }
        else if (status == "fail")
        {
            text = "用户已注销";
        }
        else if (status == "repeat")
        {
            text = "请勿重复申请";
        }
        else
        {
            text = "申请失败，请重试！";
        }
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("提示");
        msgBox->setText(text);
        connect(msgBox, &QMessageBox::buttonClicked, [=](){
            this->close();
            msgBox->deleteLater(); // 删除消息框对象
        });
        msgBox->show();
    });
}

ApplyFriendDialog::~ApplyFriendDialog()
{
    delete ui;
}

void ApplyFriendDialog::setUser(const QString &user)
{
    target_user = user;
}

void ApplyFriendDialog::on_cancelButton_clicked()
{
    close();
}


void ApplyFriendDialog::on_applyButton_clicked()
{
    const QString &text = ui->textEdit->toPlainText();
    TcpClient::GetInstance().ApplyFriend(target_user, text);
}

