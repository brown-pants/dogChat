#include "chatwidget.h"
#include "ui_chatwidget.h"
#include "chatmsgitem.h"
#include "storagemanager.h"
#include "mainwnd.h"
#include "tcpclient.h"

#include <QScrollBar>
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWidget)
{
    ui->setupUi(this);

    // 初始化表情选择窗口
    chooseEmojiWidget = new ChooseEmojiWidget(this);
    chooseEmojiWidget->hide();

    // 设置分离器鼠标样式
    ui->splitter->handle(1)->setCursor(Qt::SizeVerCursor);

    // 设置列表滑动条滚动速度
    ui->chatMsgListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->chatMsgListWidget->verticalScrollBar()->setSingleStep(10);

    // 设置输入框初始高度
    QList<int> initSizes;
    initSizes << 200 << 100;
    ui->splitter->setSizes(initSizes);

    // 解决中文后面跟着连续长串数字或字母出现默认换行的问题
    ui->msgTextEdit->setWordWrapMode(QTextOption::WrapAnywhere);

    // 限制最长文字数量
    connect(ui->msgTextEdit, &QTextEdit::textChanged, [this]() {
        const int maxLength = 4000;
        if (ui->msgTextEdit->toPlainText().length() > maxLength) {
            QTextCursor cursor = ui->msgTextEdit->textCursor();
            ui->msgTextEdit->setPlainText(ui->msgTextEdit->toPlainText().left(maxLength));
            ui->msgTextEdit->setTextCursor(cursor);
        }
    });

    // 处理表情点击信号
    connect(chooseEmojiWidget, &ChooseEmojiWidget::charEmojiClicked, [this](const QString &emoji){
        ui->msgTextEdit->insertPlainText(emoji);
    });
    // 发表情
    connect(chooseEmojiWidget, &ChooseEmojiWidget::animEmojiClicked, [this](const QString &url){
        sendFileMsg(url);
    });

    connect(ui->chatMsgListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        if (item->text() == "查看更多消息")
        {
            QListWidgetItem *top = ui->chatMsgListWidget->item(0);
            loadMsg(m_curUser);
            ui->chatMsgListWidget->scrollToItem(top, QAbstractItemView::PositionAtTop);
            ui->chatMsgListWidget->takeItem(ui->chatMsgListWidget->row(item));
        }
    });

    connect(&TcpClient::GetInstance(), &TcpClient::sig_sendMsgFail, this, [this](const QString &user, const QString &msg_id){
        qDebug() << msg_id << " send fail" << Qt::endl;
        if (m_curUser == user)
        {
            m_msgItems[msg_id]->showExclamationButton();
        }
        emit sendFail(user, msg_id);
    });
}

ChatWidget::~ChatWidget()
{
    delete ui;
    while (ui->chatMsgListWidget->count())
    {
        QListWidgetItem *item = ui->chatMsgListWidget->takeItem(0);
        delete ui->chatMsgListWidget->itemWidget(item);
        delete item;
    }
}

void ChatWidget::loadMsg(const QString &user)
{
    lastMsgTime = QDateTime();
    int cnt = StorageManager::GetInstance().chatMsgCount(user);
    int row = msgRow + 10;
    int oldRow = msgRow;

    if (row > cnt)
    {
        row = cnt;
    }

    int counter = 0;

    for (row = row - 1; row >= oldRow; row --)
    {
        msgRow ++;
        ChatMsgInfo msg = StorageManager::GetInstance().getChatMsg(user, row);
        QDateTime curTime = QDateTime::fromString(msg.time, "yyyy/MM/dd HH:mm");

        if (lastMsgTime.isNull() || lastMsgTime.secsTo(curTime) >= 300)
        {
            // 是否为今天
            if (curTime.date() == QDate::currentDate())
            {
                addTime(msg.time.split(" ")[1], counter ++);
            }
            else
            {
                addTime(msg.time, counter ++);
            }
        }
        lastMsgTime = curTime;

        // 获取头像
        const QPixmap &profile = msg.self ? MainWnd::GetInstance()->curUserProfile() : MainWnd::GetInstance()->getUserProfile(msg.user);

        ChatMsgItem *p_msgItem = nullptr;
        if (msg.type == "text")
        {
            p_msgItem = addTextMsg(msg.self, profile, msg.msg, counter ++);
        }
        else
        {
            p_msgItem = addFileMsg(msg.self, profile, msg.msg, counter ++);
        }
        m_msgItems.insert(msg.id, p_msgItem);
        if (!msg.send_succ)
        {
            p_msgItem->showExclamationButton();
        }
    }
    if (msgRow < cnt)
    {
        addLoadOld(0);
    }
}

void ChatWidget::clearMsg()
{
    m_msgItems.clear();
    while (ui->chatMsgListWidget->count() != 0)
    {
        QListWidgetItem *item = ui->chatMsgListWidget->takeItem(0);
        delete ui->chatMsgListWidget->itemWidget(item);
        delete item;
    }
}

void ChatWidget::sendFileMsg(const QString &url)
{
    const QString &time = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm");
    QString msg_id = QUuid::createUuid().toString();

    if (url.mid(0, 1) == ":")
    {// 内置表情
        ChatMsgInfo msgInfo(msg_id, time, "file", url, MainWnd::GetInstance()->curUser(), true, true);

        appendMsg(msgInfo);
        emit sendMsg(curUser(), msgInfo);

        if (m_curUser != MainWnd::GetInstance()->curUser())
        {// 发送至服务器
            TcpClient::GetInstance().SendMsg(msg_id, m_curUser, time, url, true);
        }
    }
    else
    {// 创建存储文件
        QString fileName = QFileInfo(url).fileName().remove(QRegularExpression("\\s+")); // 去除文件名空格
        // 保存文件并获取路径
        QString newUrl = StorageManager::GetInstance().saveFile(MainWnd::GetInstance()->curUser(), m_curUser, fileName, Util::FileToByteArray(url));

        ChatMsgInfo msgInfo(msg_id, time, "file", newUrl, MainWnd::GetInstance()->curUser(), true, true);
        appendMsg(msgInfo);
        emit sendMsg(curUser(), msgInfo);   // 传输文件名和Base64数据 如 sad.png ABC

        // FileToBase64很慢 多线程防止阻塞窗口渲染
        QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
        connect(watcher, &QFutureWatcher<QString>::finished, this, [watcher, msg_id, time, this]() {
            QString send_msg = watcher->result();
            if (m_curUser != MainWnd::GetInstance()->curUser())
            {// 发送至服务器
                TcpClient::GetInstance().SendMsg(msg_id, m_curUser, time, send_msg, true);
            }
            watcher->deleteLater();
        });
        QFuture<QString> future = QtConcurrent::run([=]() {
            QString send_msg = fileName + " " + Util::FileToBase64(url);
            return send_msg;
        });
        watcher->setFuture(future);
    }
}

void ChatWidget::loadMessages(const QString &user)
{
    clearMsg();
    ui->userLabel->setText(user);
    m_curUser = user;

    msgRow = 0;
    loadMsg(user);
    _LastMsgTime = lastMsgTime;
    ui->chatMsgListWidget->scrollToBottom();
}

QString ChatWidget::curUser() const
{
    return m_curUser;
}

void ChatWidget::appendMsg(ChatMsgInfo msg)
{
    QDateTime curTime = QDateTime::fromString(msg.time, "yyyy/MM/dd HH:mm");

    if (_LastMsgTime.isNull() || _LastMsgTime.secsTo(curTime) >= 300)
    {
        // 是否为今天
        if (curTime.date() == QDate::currentDate())
        {
            addTime(msg.time.split(" ")[1]);
        }
        else
        {
            addTime(msg.time);
        }
    }
    _LastMsgTime = curTime;

    // 获取头像
    const QPixmap &profile = msg.self ? MainWnd::GetInstance()->curUserProfile() : MainWnd::GetInstance()->getUserProfile(msg.user);

    ChatMsgItem *p_msgItem = nullptr;
    if (msg.type == "text")
    {
        p_msgItem = addTextMsg(msg.self, profile, msg.msg);
    }
    else
    {
        p_msgItem = addFileMsg(msg.self, profile, msg.msg);
    }
    m_msgItems.insert(msg.id, p_msgItem);
    if (!msg.send_succ)
    {
        p_msgItem->showExclamationButton();
    }

    ui->chatMsgListWidget->scrollToBottom();
}

ChatMsgItem *ChatWidget::addTextMsg(bool myMsg, const QPixmap &profile, const QString &text, int row)
{
    QListWidgetItem *item = new QListWidgetItem();

    ChatMsgItem *pItemWidget = new ChatMsgItem(myMsg, profile, item, ui->chatMsgListWidget);
    pItemWidget->setText(text);

    if (row < 0)
    {
        ui->chatMsgListWidget->addItem(item);
    }
    else
    {
        ui->chatMsgListWidget->insertItem(row, item);
    }
    ui->chatMsgListWidget->setItemWidget(item, pItemWidget);
    return pItemWidget;
}

ChatMsgItem *ChatWidget::addFileMsg(bool myMsg, const QPixmap &profile, const QString &url, int row)
{
    QListWidgetItem *item = new QListWidgetItem();

    ChatMsgItem *pItemWidget = new ChatMsgItem(myMsg, profile, item, ui->chatMsgListWidget);
    pItemWidget->setFile(url);

    if (row < 0)
    {
        ui->chatMsgListWidget->addItem(item);
    }
    else
    {
        ui->chatMsgListWidget->insertItem(row, item);
    }
    ui->chatMsgListWidget->setItemWidget(item, pItemWidget);
    return pItemWidget;
}

void ChatWidget::addTime(const QString &time, int row)
{
    QListWidgetItem* item = nullptr;
    if (row < 0)
    {
        ui->chatMsgListWidget->addItem(time);
        item = ui->chatMsgListWidget->item(ui->chatMsgListWidget->count() - 1);
    }
    else
    {
        ui->chatMsgListWidget->insertItem(row, time);
        item = ui->chatMsgListWidget->item(row);
    }
    item->setTextAlignment(Qt::AlignCenter);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
    item->setForeground(QBrush(Qt::gray));
}

void ChatWidget::addLoadOld(int row)
{
    QListWidgetItem* item = nullptr;
    ui->chatMsgListWidget->insertItem(row, "查看更多消息");
    item = ui->chatMsgListWidget->item(row);
    item->setTextAlignment(Qt::AlignCenter);
    item->setForeground(QBrush(QColor(50, 120, 180)));
}

void ChatWidget::on_sendButton_clicked()
{
    const QString &text = ui->msgTextEdit->document()->toPlainText();
    QString msg_id = QUuid::createUuid().toString();

    if (text != "")
    {
        const QString &time = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm");
        ChatMsgInfo msgInfo(msg_id, time, "text", text, MainWnd::GetInstance()->curUser(), true, true);
        appendMsg(msgInfo);
        ui->msgTextEdit->clear();

        if (m_curUser != MainWnd::GetInstance()->curUser())
        {// 发送至服务器
            TcpClient::GetInstance().SendMsg(msg_id, m_curUser, time, text, false);
        }

        emit sendMsg(curUser(), msgInfo);
    }
}

void ChatWidget::on_emojiButton_clicked()
{
    QPoint offset(ui->emojiButton->width() / 2 - chooseEmojiWidget->width() / 2, -chooseEmojiWidget->height());
    QPoint globalPos = ui->emojiButton->mapToGlobal(ui->emojiButton->pos()) + offset;
    chooseEmojiWidget->move(globalPos);
    chooseEmojiWidget->show();
}

void ChatWidget::on_imgButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择图像", "", "图像文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (fileName != "")
    {
        sendFileMsg(fileName);
    }
}

void ChatWidget::on_fileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择文件", "");
    if (fileName != "")
    {
        sendFileMsg(fileName);
    }
}

