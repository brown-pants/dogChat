#include "loginwnd.h"
#include "ui_loginwnd.h"
#include "storagemanager.h"
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QMovie>
#include <QPaintEvent>
#include <QComboBox>
#include <QAbstractItemView>
#include <QFileDialog>

LoginWnd::LoginWnd(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWnd), rotation(0), LockPixmap(":/res/img/Lock.png"), LockOpenPixmap(":/res/img/LockOpen.png")
{
    ui->setupUi(this);

    // 窗口无控制栏且透明
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 窗口可拖拽
    setProperty("canMove", true);

    // 设置外边框阴影参数
    int shadowWidth = 20;
    setProperty("shadowWidth", shadowWidth);
    layout()->setContentsMargins(shadowWidth, shadowWidth, shadowWidth, shadowWidth);

    // 设置外边框阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(50, 50, 50));
    shadow->setBlurRadius(shadowWidth);
    ui->shadowWidget->setGraphicsEffect(shadow);

    // 加载动图
    topMovie = new QMovie(":/res/gif/loginbg.gif");
    connect(topMovie, &QMovie::frameChanged, [this](){
        ui->topWidget->update();
    });
    topMovie->start();

    // 设置事件过滤器(渲染上方动图)
    ui->topWidget->installEventFilter(this);

    // 设置固定尺寸
    setFixedSize(500, 400);
    ui->shadowWidget->setFixedHeight(height() - 2 * shadowWidth);
    setFixedHeight(600); // 避免旋转时左右下角被遮挡
    ui->topWidget->setFixedHeight(130);

    // 设置控制栏
    ui->controlWidget->setControlWnd(this);
    ui->controlWidget->hideMaximizeButton();

    // 设置输入框下方横线颜色
    connect(qApp, &QApplication::focusChanged, this, &LoginWnd::onFocusChanged);

    // 设置头像
    profileButton = new QPushButton(ui->shadowWidget);
    profileButton->resize(80, 80);
    profileButton->move(width() / 2 - shadowWidth - profileButton->width() / 2, ui->topWidget->height() - profileButton->height() / 2);
    profileButton->setIconSize(profileButton->size());

    // 设置头像阴影
    QGraphicsDropShadowEffect *profileShadow = new QGraphicsDropShadowEffect(this);
    profileShadow->setOffset(0, 0);
    profileShadow->setColor(QColor(50, 50, 50));
    profileShadow->setBlurRadius(shadowWidth);
    profileButton->setGraphicsEffect(profileShadow);

    // 设置事件过滤器(渲染圆形头像)
    profileButton->installEventFilter(this);

    // 设置翻转动画
    rotateAnim = new QPropertyAnimation(this, "rotation");

    // 动画结束后正常显示
    connect(rotateAnim, &QPropertyAnimation::finished, [this, shadowWidth](){
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
        shadow->setOffset(0, 0);
        shadow->setColor(QColor(50, 50, 50));
        shadow->setBlurRadius(shadowWidth);
        ui->shadowWidget->setGraphicsEffect(shadow);
        ui->shadowWidget->show();
        // 设置头像按钮槽函数
        if (ui->stackedWidget->currentWidget() == ui->loginPage)
        {
            disconnect(profileButton, &QPushButton::pressed, this, &LoginWnd::chooseProfile);
        }
        else if (ui->stackedWidget->currentWidget() == ui->registerPage)
        {
            connect(profileButton, &QPushButton::pressed, this, &LoginWnd::chooseProfile);
        }
    });

    // 设置事件过滤器(渲染翻转效果)
    installEventFilter(this);

    // 设置账号下拉框
    ui->accountComboBox->view()->setMaximumHeight(100);
    ui->accountComboBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    ui->accountComboBox->addItems(StorageManager::GetInstance().users());

    // 账号改变时更改头像
    connect(ui->accountComboBox, &QComboBox::currentTextChanged, [this](const QString &curText){
        QPixmap *profile = StorageManager::GetInstance().profile(curText);
        if (profile)
        {
            profileButton->setIcon(QIcon(*profile));
        }
        else
        {
            profileButton->setIcon(QIcon());
        }
    });
    emit ui->accountComboBox->currentTextChanged(ui->accountComboBox->currentText());

    // 设置事件过滤器(下拉列表位置调整)
    ui->accountComboBox->view()->installEventFilter(this);

    // 显示登录页
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}

LoginWnd::~LoginWnd()
{
    delete ui;
}

bool LoginWnd::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->topWidget && event->type() == QEvent::Paint)
    {
        QPainter painter(ui->topWidget);
        QRect topRect = ui->topWidget->rect();
        QPainterPath path;
        int radius = 5;

        // 设置左右上角圆弧
        path.moveTo(topRect.bottomLeft());
        path.lineTo(topRect.topLeft().x(), topRect.topLeft().y() + radius);
        path.quadTo(topRect.topLeft(), QPoint(topRect.topLeft().x() + radius, topRect.topLeft().y()));
        path.lineTo(topRect.topRight().x() - radius, topRect.topRight().y());
        path.quadTo(topRect.topRight(), QPoint(topRect.topRight().x(), topRect.topRight().y() + radius));
        path.lineTo(topRect.bottomRight().x() + 2, topRect.bottomRight().y() + 2);
        path.lineTo(topRect.bottomLeft().x(), topRect.bottomLeft().y() + 2);
        painter.setClipPath(path);

        // 绘制动图
        painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿
        painter.drawPixmap(topRect, topMovie->currentPixmap());
    }
    else if (obj == profileButton && event->type() == QEvent::Paint)
    {
        QPainter painter(profileButton);
        painter.setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿
        QRect porfileRect = profileButton->rect();

        // 1. 绘制圆形背景
        painter.setOpacity(0.3);
        painter.setBrush(QBrush(qRgb(150, 218, 248)));  // 背景色
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(porfileRect);

        // 2. 创建圆形裁剪路径
        QPainterPath clipPath;
        clipPath.addEllipse(porfileRect);
        painter.setClipPath(clipPath);  // 设置裁剪区域

        // 3. 绘制图标（超出部分自动被裁剪）
        if (!porfileRect.isNull())
        {
            QPixmap pixmap = profileButton->icon().pixmap(profileButton->iconSize());
            painter.setOpacity(1.0);
            painter.drawPixmap(porfileRect, pixmap, pixmap.rect());
        }
        return true;
    }
    else if (obj == this && event->type() == QEvent::Paint)
    {
        if (rotateAnim->state() == QPropertyAnimation::Running)
        {
            QPainter painter(this);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);

            QPixmap pixmap(ui->shadowWidget->size());
            QTransform trans;
            trans.translate(width() / 2, 0);

            // 登录页
            if (rotation < 90 && rotation > -90)
            {
                ui->stackedWidget->setCurrentWidget(ui->loginPage);
                trans.rotate(rotation, Qt::YAxis);
                ui->loginLeftVLayout->addWidget(ui->toRegisterButton);
                ui->loginRightGLayout->addWidget(ui->toSettingButton, 1, 1);
                emit ui->accountComboBox->currentTextChanged(ui->accountComboBox->currentText());
                profileButton->show();
            }
            // 注册页
            else if (rotation >= 90)
            {
                ui->stackedWidget->setCurrentWidget(ui->registerPage);
                trans.rotate(rotation + 180, Qt::YAxis);
                ui->registerLeftVLayout->addWidget(ui->toLoginButton);
                ui->registerRightGLayout->addWidget(ui->toSettingButton, 1, 2);
                profileButton->setIcon(QIcon(registerProfile));
                profileButton->show();
            }
            // 设置页
            else
            {
                ui->stackedWidget->setCurrentWidget(ui->settingPage);
                trans.rotate(rotation + 180, Qt::YAxis);
                ui->settingLeftVLayout->addWidget(ui->toLoginButton);
                ui->settingRightGLayout->addWidget(ui->toRegisterButton, 1, 2);
                profileButton->hide();
            }

            int shadowWidth = property("shadowWidth").toInt();

            ui->shadowWidget->render(&pixmap);

            painter.setTransform(trans);
            painter.drawPixmap(-width() / 2 + shadowWidth, shadowWidth, pixmap);
        }
    }
    else if (obj == ui->accountComboBox->view() && event->type() == QEvent::Paint)
    {
        ui->accountComboBox->view()->move(0, 2);
    }
    return QWidget::eventFilter(obj, event);
}

void LoginWnd::onFocusChanged(QWidget *old, QWidget *now)
{
    static QVector<QWidget *> lineEdits = {
        ui->accountComboBox, ui->passwordEdit, ui->userEdit, ui->pwdEdit, ui->rePwdEdit, ui->serverIpEdit, ui->serverPortEdit
    };
    static QVector<QWidget *> widgets = {
        ui->accountWidget, ui->passwordWidget, ui->userWidget, ui->pwdWidget, ui->rePwdWidget, ui->serverIpWidget, ui->serverPortWidget
    };
    for (int i = 0; i < lineEdits.size(); i ++)
    {
        if (now == lineEdits[i])
        {
            if (now == ui->passwordEdit)
            {
                ui->passwordIconLabel->setPixmap(LockOpenPixmap);
            }
            widgets[i]->setStyleSheet("#" + widgets[i]->objectName() + "{border-bottom: 1px solid rgb(100, 150, 200);}");
        }
        else if (old == lineEdits[i])
        {
            if (old == ui->passwordEdit)
            {
                ui->passwordIconLabel->setPixmap(LockPixmap);
            }
            widgets[i]->setStyleSheet("#" + widgets[i]->objectName() + "{border-bottom: 1px solid rgb(180, 180, 180);}");
        }
    }
}

void LoginWnd::on_toRegisterButton_clicked()
{
    ui->shadowWidget->hide();
    ui->shadowWidget->setGraphicsEffect(nullptr);
    rotateAnim->setStartValue(rotation);
    rotateAnim->setEndValue(180);
    rotateAnim->setDuration(800);
    rotateAnim->start();
}

void LoginWnd::on_toLoginButton_clicked()
{
    ui->shadowWidget->hide();
    ui->shadowWidget->setGraphicsEffect(nullptr);
    rotateAnim->setStartValue(rotation);
    rotateAnim->setEndValue(0);
    rotateAnim->setDuration(800);
    rotateAnim->start();
}

void LoginWnd::on_toSettingButton_clicked()
{
    ui->shadowWidget->hide();
    ui->shadowWidget->setGraphicsEffect(nullptr);
    rotateAnim->setStartValue(rotation);
    rotateAnim->setEndValue(-180);
    rotateAnim->setDuration(800);
    rotateAnim->start();
}

void LoginWnd::chooseProfile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择图像", "", "图像文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    registerProfile.load(fileName);
    profileButton->setIcon(QIcon(registerProfile));
}


int LoginWnd::getRotation() const
{
    return rotation;
}

void LoginWnd::setRotation(int newRotation)
{
    rotation = newRotation;
    update();
}
