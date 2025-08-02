#include "loginwnd.h"
#include "ui_loginwnd.h"
#include "storagemanager.h"
#include "tcpclient.h"
#include "util.h"
#include "mainwnd.h"
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QMovie>
#include <QPaintEvent>
#include <QComboBox>
#include <QAbstractItemView>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QThread>

LoginWnd::LoginWnd(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWnd), rotation(0), LockPixmap(":/res/img/Lock.png"), LockOpenPixmap(":/res/img/LockOpen.png"), servConnSucc(false), topRounded(false)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/res/icon/anim7.ico"));

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
    connect(topMovie, &QMovie::frameChanged, this, [this](){
        ui->topWidget->update();
    });
    topMovie->start();

    // 设置事件过滤器(渲染上方动图)
    ui->topWidget->installEventFilter(this);

    // 设置固定尺寸
    setFixedSize(500, 400);
    ui->shadowWidget->setFixedHeight(height() - 2 * shadowWidth);
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
    connect(rotateAnim, &QPropertyAnimation::finished, this, [this, shadowWidth](){
        setFixedSize(500, 400);
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
        shadow->setOffset(0, 0);
        shadow->setColor(QColor(50, 50, 50));
        shadow->setBlurRadius(shadowWidth);
        ui->shadowWidget->setGraphicsEffect(shadow);
        ui->shadowWidget->show();
        // 设置头像按钮槽函数
        if (ui->stackedWidget->currentWidget() != ui->registerPage)
        {
            disconnect(profileButton, &QPushButton::pressed, this, &LoginWnd::chooseProfile);
        }
        else
        {
            connect(profileButton, &QPushButton::pressed, this, &LoginWnd::chooseProfile);
        }
    });

    // slide动画
    slideAnim = new QVariantAnimation(this);
    connect(slideAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value){
        int height = value.toInt();
        ui->topWidget->setFixedHeight(height);
    });
    connect(slideAnim, &QVariantAnimation::finished, this, [this](){
        ui->stackedWidget->setEnabled(true);
        topRounded = !topRounded;
    });

    loggingLabel = new QLabel(this);
    loggingLabel->hide();
    loggingTimer = new QTimer(this);
    connect(loggingTimer, &QTimer::timeout, this, [this](){
        int dotCount = loggingLabel->text().count('.');
        if (dotCount == 3)
        {
            loggingLabel->setText("登录中.");
        }
        else loggingLabel->setText(loggingLabel->text() + '.');
    });

    rtnBtn = new QPushButton("返回", this);
    rtnBtn->resize(100, 50);
    rtnBtn->move(width() / 2 - rtnBtn->width() / 2, height() / 2 + 50);
    rtnBtn->setStyleSheet("QPushButton {color: rgb(255, 255, 255);} "
                          "QPushButton:hover {color: rgb(235, 235, 235);} "
                          "QPushButton:pressed {color: rgb(215, 215, 215);} ");
    rtnBtn->hide();
    connect(rtnBtn, &QPushButton::clicked, this, [this](){
        slideAnim->setDuration(500);
        slideAnim->setStartValue(ui->topWidget->height());
        slideAnim->setEndValue(130);
        slideAnim->start();
        loggingLabel->hide();
        rtnBtn->hide();
        ui->stackedWidget->setEnabled(false);
    });

    // 设置事件过滤器(渲染翻转效果)
    installEventFilter(this);

    // 设置账号下拉框
    ui->accountComboBox->view()->setMaximumHeight(100);
    ui->accountComboBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
    ui->accountComboBox->addItems(StorageManager::GetInstance().users());
    ui->accountComboBox->setCompleter(nullptr);  // 禁用自动补全

    // 账号改变时更改头像
    connect(ui->accountComboBox, &QComboBox::currentTextChanged, this, [this](const QString &curText){
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

    // 设置页
    ui->serverIpEdit->setText(StorageManager::GetInstance().serverIp());
    ui->serverPortEdit->setText(StorageManager::GetInstance().serverPort());

    // 保存按钮
    connect(ui->settingButton, &QPushButton::clicked, this, [this](){
        StorageManager::GetInstance().setServerIp(ui->serverIpEdit->text());
        StorageManager::GetInstance().setServerPort(ui->serverPortEdit->text());
        StorageManager::GetInstance().saveConfig();
        // 重新连接TCP服务器
        servConnSucc = false;
        TcpClient::GetInstance().tcp_disconnect();
        TcpClient::GetInstance().tcp_connect();
        QMessageBox::information(this, "消息", "保存成功");
    });

    // 登陆注册槽函数
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWnd::login);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginWnd::regist);

    connect(&TcpClient::GetInstance(), &TcpClient::connected, this, [this](){
        servConnSucc = true;
    });

    // 注册完成
    connect(&TcpClient::GetInstance(), &TcpClient::sig_regist_over, this, [this](const QString &status){
        if (status == "success")
        {
            QMessageBox::information(this, "消息", "注册成功！");
        }
        else if (status == "fail")
        {
            QMessageBox::information(this, "消息", "用户名已存在，注册失败！");
        }
        else
        {
            QMessageBox::information(this, "消息", "服务器异常，注册失败！");
        }
        ui->registerButton->setEnabled(true);
    });

    // 登录完成
    connect(&TcpClient::GetInstance(), &TcpClient::sig_login_over, this, [this](const QString &status, const QString &profile){
        if (status == "success")
        {
            // 子线程加载
            QThread *loadThread = new QThread(this);
            MainWndLoader *loader = new MainWndLoader;
            loader->moveToThread(loadThread);

            // 加载线程开始
            connect(loadThread, &QThread::started, loader, [this, loader, profile]() {
                // 获取用户名
                QString user = ui->accountComboBox->currentText();

                // 设置头像
                QPixmap profilePixmap;
                QByteArray profileData = QByteArray::fromBase64(profile.toUtf8());
                profilePixmap.loadFromData(profileData);
                profileButton->setIcon(QIcon(profilePixmap));
                StorageManager::GetInstance().addUser(user, profileData);

                // 加载主窗口数据
                loader->load(user, profilePixmap);
            });

            // 加载结束 关闭子线程
            connect(loader, &MainWndLoader::finished, loadThread, &QThread::quit);

            // 线程结束 关闭登录页面
            connect(loadThread, &QThread::finished, this, [this, loader](){
                loggingTimer->stop();
                loggingLabel->setText("登录成功!");
                loggingLabel->adjustSize();
                loggingLabel->move(width() / 2 - loggingLabel->width() / 2, height() / 2);

                // 重启定时器
                disconnect(loggingTimer);
                connect(loggingTimer, &QTimer::timeout, this, [this, loader](){
                    loggingTimer->stop();
                    delete loader;
                    close();
                    MainWnd::GetInstance()->show();
                });
                loggingTimer->start(2000);
            });

            loadThread->start();
        }
        else if (status == "fail")
        {
            qDebug() << "login fail" << Qt::endl;
            loggingTimer->stop();
            loggingLabel->setText("用户名或密码错误!");
            loggingLabel->setStyleSheet("color: red");
            loggingLabel->adjustSize();
            loggingLabel->move(width() / 2 - loggingLabel->width() / 2, height() / 2);
            // 显示返回按钮
            rtnBtn->show();
        }
        else
        {
            qDebug() << "login error" << Qt::endl;
            loggingTimer->stop();
            loggingLabel->setText("服务器异常!");
            loggingLabel->setStyleSheet("color: red");
            loggingLabel->adjustSize();
            loggingLabel->move(width() / 2 - loggingLabel->width() / 2, height() / 2);
            // 显示返回按钮
            rtnBtn->show();
        }
    });

    // 显示登录页
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
}

LoginWnd::~LoginWnd()
{
    qDebug() << "~LoginWnd()" << Qt::endl;
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
        if (topRounded)
        {
            path.addRoundedRect(topRect, radius, radius);
        }
        else
        {
            path.moveTo(topRect.bottomLeft());
            path.lineTo(topRect.topLeft().x(), topRect.topLeft().y() + radius);
            path.quadTo(topRect.topLeft(), QPoint(topRect.topLeft().x() + radius, topRect.topLeft().y()));
            path.lineTo(topRect.topRight().x() - radius, topRect.topRight().y());
            path.quadTo(topRect.topRight(), QPoint(topRect.topRight().x(), topRect.topRight().y() + radius));
            path.lineTo(topRect.bottomRight().x() + 2, topRect.bottomRight().y() + 2);
            path.lineTo(topRect.bottomLeft().x(), topRect.bottomLeft().y() + 2);
        }
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
    setFixedHeight(600); // 避免旋转时左右下角被遮挡
    ui->shadowWidget->hide();
    ui->shadowWidget->setGraphicsEffect(nullptr);
    rotateAnim->setStartValue(rotation);
    rotateAnim->setEndValue(180);
    rotateAnim->setDuration(800);
    rotateAnim->start();
    // 恢复设置数据
    if (ui->stackedWidget->currentWidget() == ui->settingPage)
    {
        ui->serverIpEdit->setText(StorageManager::GetInstance().serverIp());
        ui->serverPortEdit->setText(StorageManager::GetInstance().serverPort());
    }
}

void LoginWnd::on_toLoginButton_clicked()
{
    setFixedHeight(600); // 避免旋转时左右下角被遮挡
    ui->shadowWidget->hide();
    ui->shadowWidget->setGraphicsEffect(nullptr);
    rotateAnim->setStartValue(rotation);
    rotateAnim->setEndValue(0);
    rotateAnim->setDuration(800);
    rotateAnim->start();
    // 恢复设置数据
    if (ui->stackedWidget->currentWidget() == ui->settingPage)
    {
        ui->serverIpEdit->setText(StorageManager::GetInstance().serverIp());
        ui->serverPortEdit->setText(StorageManager::GetInstance().serverPort());
    }
}

void LoginWnd::on_toSettingButton_clicked()
{
    setFixedHeight(600); // 避免旋转时左右下角被遮挡
    ui->shadowWidget->hide();
    ui->shadowWidget->setGraphicsEffect(nullptr);
    rotateAnim->setStartValue(rotation);
    rotateAnim->setEndValue(-180);
    rotateAnim->setDuration(800);
    rotateAnim->start();
}

void LoginWnd::chooseProfile()
{
    profilePath = QFileDialog::getOpenFileName(this, "选择图像", "", "图像文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    registerProfile.load(profilePath);
    profileButton->setIcon(QIcon(registerProfile));
}

void LoginWnd::login()
{
    if (!servConnSucc)
    {
        QMessageBox::critical(this, "错误", "服务器连接失败");
        return;
    }
    if (ui->accountComboBox->currentText().isEmpty() || ui->passwordEdit->text().isEmpty())
    {
        QMessageBox::critical(this, "错误", "用户名或密码为空");
        return;
    }
    TcpClient::GetInstance().Login(ui->accountComboBox->currentText(), ui->passwordEdit->text());
    // 动画
    ui->shadowVLayout->setEnabled(false);
    ui->stackedWidget->setEnabled(false);
    ui->stackedWidget->stackUnder(ui->topWidget);
    slideAnim->setDuration(500);
    slideAnim->setStartValue(ui->topWidget->height());
    slideAnim->setEndValue(ui->shadowWidget->height());
    slideAnim->start();
    loggingLabel->setText("登录中...");
    loggingTimer->start(800);
    loggingLabel->setStyleSheet("color: rgb(255, 255, 255);");
    loggingLabel->move(width() / 2 - 24, height() / 2);
    loggingLabel->show();
}

void LoginWnd::regist()
{
    if (!servConnSucc)
    {
        QMessageBox::critical(this, "错误", "服务器连接失败");
        return;
    }
    if (ui->pwdEdit->text() != ui->rePwdEdit->text())
    {
        QMessageBox::critical(this, "错误", "两次密码不同");
        return;
    }
    if (registerProfile.isNull())
    {
        QMessageBox::critical(this, "错误", "请设置头像");
        return;
    }
    QRegularExpression userRegExp("^[\\p{L}0-9]{1,20}$");
    const QString &user = ui->userEdit->text();
    if (!userRegExp.match(user).hasMatch())
    {
        QMessageBox::critical(this, "错误", "用户名由20以内非零个非特殊字符组成");
        return;
    }
    QRegularExpression pwdRegExp("^[a-zA-Z0-9]{1,20}$");
    const QString &pwd = ui->pwdEdit->text();
    if (!pwdRegExp.match(pwd).hasMatch())
    {
        QMessageBox::critical(this, "错误", "密码由20以内非零个英文字符组成");
        return;
    }
    QString profileData = Util::FileToBase64(profilePath);
    if (profileData.size() > 2147483648)
    {
        QMessageBox::critical(this, "错误", "图片不得超过2G");
        return;
    }
    // 注册
    ui->registerButton->setEnabled(false);
    TcpClient::GetInstance().Regist(user, pwd, profileData);
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
