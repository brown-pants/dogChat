#include "finduserwidget.h"
#include "ui_finduserwidget.h"
#include "mainwnd.h"
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>

FindUserWidget::FindUserWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FindUserWidget), applyDialog(nullptr)
{
    ui->setupUi(this);

    setFixedSize(300, 200);

    // 窗口无控制栏 点击其他窗口时关闭 且 透明
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置外边框阴影参数
    int shadowWidth = 20;
    layout()->setContentsMargins(shadowWidth, shadowWidth, shadowWidth, shadowWidth);

    //设置外边框阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(120, 120, 120));
    shadow->setBlurRadius(shadowWidth);
    ui->shadowWidget->setGraphicsEffect(shadow);

    ui->profileLabel->installEventFilter(this);
}

FindUserWidget::~FindUserWidget()
{
    delete ui;
}

void FindUserWidget::setInfo(const QString &name, const QPixmap &profile, bool isFriend)
{
    m_user = name;
    ui->nameLabel->setText(name);
    ui->profileLabel->setPixmap(profile);
    ui->profileLabel->setScaledContents(true);

    if (isFriend)
    {
        ui->button->setText("发消息");
    }
    else
    {
        ui->button->setText("加好友");
    }

    // 超出文本范围显示...
    QFontMetrics metrics(ui->nameLabel->font());
    QString elidedText = metrics.elidedText(name, Qt::ElideRight, ui->nameLabel->width());
    ui->nameLabel->setText(elidedText);
    if(elidedText != name)
    {
        ui->nameLabel->setToolTip(name);
    }
}

bool FindUserWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (m_user != MainWnd::GetInstance()->curUser() || parentWidget() != MainWnd::GetInstance())
    {
        return false;
    }
    static bool pressed = false;
    QMouseEvent *mouse_event = static_cast<QMouseEvent *>(e);

    if (e->type() == QEvent::MouseButtonPress && mouse_event->button() == Qt::LeftButton)
    {
        pressed = true;
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        if (pressed)
        {
            QString profilePath = QFileDialog::getOpenFileName(parentWidget(), "选择图像", "", "图像文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
            if (profilePath != "")
            {
                emit profileChanged(profilePath);
            }
        }
        pressed = false;
    }
    return QWidget::eventFilter(obj, e);
}

void FindUserWidget::on_button_clicked()
{
    if (ui->button->text() == "加好友")
    {
        if (applyDialog == nullptr)
        {
            applyDialog = new ApplyFriendDialog(MainWnd::GetInstance());
        }
        applyDialog->setUser(m_user);
        applyDialog->show();
    }
    else
    {
        emit toChat(m_user);
        hide();
    }
}

