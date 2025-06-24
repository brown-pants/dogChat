#include "chatwidget.h"
#include "ui_chatwidget.h"
#include "chatmsgitem.h"

#include <QScrollBar>
#include <QPixmap>
#include <QFileDialog>

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
    connect(chooseEmojiWidget, &ChooseEmojiWidget::animEmojiClicked, [this](const QString &url){
        addFileMsg(true, QPixmap("D:\\code\\MyChat\\xjm.gif"), url);
        ui->chatMsgListWidget->scrollToBottom();
    });


    //-----------------------------

    addTextMsg(false, QPixmap("D:\\code\\MyChat\\xjm.gif"), "你好呀\n我是小鸡毛1111n我是小鸡毛1111n我是小鸡毛1111n我是小鸡毛1111n我是小鸡毛1111n我是小鸡毛1111n我是小鸡毛1111n我是小鸡毛1111n我是小鸡毛1111n我是小鸡毛1111");

    //-----------------------------
}

ChatWidget::~ChatWidget()
{
    delete ui;
}

void ChatWidget::addTextMsg(bool myMsg, const QPixmap &profile, const QString &text)
{
    QListWidgetItem *item = new QListWidgetItem(ui->chatMsgListWidget);
    ChatMsgItem *pItemWidget = new ChatMsgItem(myMsg, profile, item, ui->chatMsgListWidget);
    pItemWidget->setText(text);
    ui->chatMsgListWidget->addItem(item);
    ui->chatMsgListWidget->setItemWidget(item, pItemWidget);
}

void ChatWidget::addFileMsg(bool myMsg, const QPixmap &profile, const QString &url)
{
    QListWidgetItem *item = new QListWidgetItem(ui->chatMsgListWidget);
    ChatMsgItem *pItemWidget = new ChatMsgItem(myMsg, profile, item, ui->chatMsgListWidget);
    pItemWidget->setFile(url);
    ui->chatMsgListWidget->addItem(item);
    ui->chatMsgListWidget->setItemWidget(item, pItemWidget);
}

void ChatWidget::on_sendButton_clicked()
{
    const QString &text = ui->msgTextEdit->document()->toPlainText();
    if (text != "")
    {
        addTextMsg(true, QPixmap("D:\\code\\MyChat\\xjm.gif"), ui->msgTextEdit->document()->toPlainText());
        ui->chatMsgListWidget->scrollToBottom();
        ui->msgTextEdit->clear();
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
    QString fileName = QFileDialog::getOpenFileName(this, "选择图像", "", "图像文件 (*.png; *.jpg; *.jpeg; *.bmp; *.gif)");
    if (fileName != "")
    {
        addFileMsg(true, QPixmap("D:\\code\\MyChat\\xjm.gif"), fileName);
        ui->chatMsgListWidget->scrollToBottom();
    }
}

void ChatWidget::on_fileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择文件", "");
    if (fileName != "")
    {
        addFileMsg(true, QPixmap("D:\\code\\MyChat\\xjm.gif"), fileName);
        ui->chatMsgListWidget->scrollToBottom();
    }
}

