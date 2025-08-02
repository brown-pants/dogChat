#include "chatmsgitem.h"
#include "ui_chatmsgitem.h"
#include "filemsgwidget.h"

#include <QMovie>
#include <QPainter>
#include <QFileInfo>
#include <QTextEdit>
#include <QTextBlock>
#include <QPaintEvent>
#include <QMessageBox>
#include <QDesktopServices>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QPainterPath>
#include <QMenu>

QIcon ChatMsgItem::exclamationIcon;

ChatMsgItem::ChatMsgItem(bool myMsg, const QPixmap &profile, QListWidgetItem *listWidgetItem, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatMsgItem), hover(false), myMsg(myMsg), bubbleColor(201, 231, 255), listWidgetItem(listWidgetItem)
{
    ui->setupUi(this);

    // 加载图标(避免重复加载)
    if (exclamationIcon.isNull())
    {
        exclamationIcon.addPixmap(QPixmap(":/res/img/exclamation.png"));
    }

    // 设置图标
    ui->exclamationButton->setIcon(exclamationIcon);
    ui->exclamationButton->setIconSize(QSize(16, 16));
    ui->exclamationButton->setFixedSize(QSize(16, 16));

    // 如果是自己发送的消息则重新布局
    if (myMsg)
    {
        ui->mainHLayout->addWidget(ui->exclamationWidget);
        ui->mainHLayout->addWidget(ui->msgWidget);
        ui->mainHLayout->addWidget(ui->profileWidget);
    }
    else
    {
        // 设置对方气泡颜色
        bubbleColor = QColor(228, 231, 235);
    }

    // 设置头像
    ui->profileLabel->setPixmap(profile);
    // 隐藏感叹
    ui->exclamationButton->hide();
}

ChatMsgItem::~ChatMsgItem()
{
    delete ui;
}

void ChatMsgItem::setText(const QString &text)
{
    msgType = TextMsg;
    QTextEdit *textEdit = new QTextEdit(this);

    // 调整布局和样式
    ui->msgHLayout->addWidget(textEdit);
    ui->msgWidget->setStyleSheet(QString("#msgWidget, #msgWidget QTextEdit { background-color: rgb(%1, %2, %3); border-radius: 10px; }").arg(bubbleColor.red()).arg(bubbleColor.green()).arg(bubbleColor.blue()));

    // 隐藏滑块
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置文本
    textEdit->setPlainText(text);
    textEdit->setReadOnly(true);

    // 设置字体大小
    QFont font;
    font.setPointSize(10);
    textEdit->setFont(font);

    // 解决中文后面跟着连续长串数字或字母出现默认换行的问题
    textEdit->setWordWrapMode(QTextOption::WrapAnywhere);

    // 计算最大宽度 (最长一行的宽度)
    textEdit->document()->adjustSize();
    QTextDocument *doc = textEdit->document();
    QFontMetrics fm(textEdit->font());
    int maxWidth = 0;

    // 遍历所有文本块（行）获取最长行
    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next())
    {
        QString lineText = block.text();
        int lineWidth = fm.horizontalAdvance(lineText); // 计算该行的像素宽度
        if (lineWidth > maxWidth)
        {
            maxWidth = lineWidth;
        }
    }

    // 设置最大宽度
    ui->msgWidget->setMaximumWidth(maxWidth + ui->msgWidget->contentsMargins().left() + ui->msgWidget->contentsMargins().right() + 30);

    // 设置文本高度属性
    setProperty("lastDocHeight", 0);

    // 当文本高度改变时调整布局
    connect(textEdit->document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged, [this, textEdit]() {
        int docHeight = textEdit->document()->size().height();
        int lastDocHeight = property("lastDocHeight").toInt();
        if (docHeight != lastDocHeight)
        {
            textEdit->setFixedHeight(docHeight);
            ui->msgWidget->setFixedHeight(docHeight + ui->msgHLayout->contentsMargins().top() + ui->msgHLayout->contentsMargins().bottom());
            listWidgetItem->setSizeHint(QSize(0, ui->msgWidget->height() + ui->mainHLayout->contentsMargins().top() + ui->mainHLayout->contentsMargins().bottom() + 2));
        }
        setProperty("lastDocHeight", docHeight);
    });
}

void ChatMsgItem::setFile(const QString &file_url)
{
    QFileInfo fileInfo(file_url);
    QString suffix = fileInfo.suffix();

    // 静态图像文件
    if (suffix == "png" || suffix == "jpg"  || suffix == "jpeg" || suffix == "bmp")
    {
        msgType = ImageMsg;
        QLabel *imgLabel = new QLabel(this);
        QPixmap pixmap(file_url);

        // 最大图像宽度
        const int maxWidth = 300;
        if (pixmap.width() > maxWidth)
        {
            pixmap = pixmap.scaledToWidth(maxWidth, Qt::SmoothTransformation);
        }

        // 添加图像并调整大小
        imgLabel->setPixmap(pixmap);
        imgLabel->setFixedSize(pixmap.size());
        ui->msgHLayout->addWidget(imgLabel);
        adjustSize();
        listWidgetItem->setSizeHint(size());
    }
    // 动态图像文件
    else if (suffix == "gif")
    {
        msgType = ImageMsg;
        QLabel *gifLabel = new QLabel(this);
        QMovie *movie = new QMovie(file_url, QByteArray(), this);
        movie->start();

        // 最大动图宽度
        const int maxWidth = 100;
        QSize movieSize = movie->frameRect().size();
        if (movieSize.width() > maxWidth)
        {
            movieSize = QSize(maxWidth, (float)movieSize.height() / movieSize.width() * maxWidth);
            movie->setScaledSize(movieSize);
        }

        // 使用高质量缩放但不中断动画
        connect(movie, &QMovie::frameChanged, [=]{
            gifLabel->setPixmap(movie->currentPixmap().scaled(
                movieSize.width(),
                movieSize.height(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            ));
        });

        // 调整大小
        gifLabel->setFixedSize(movieSize);
        ui->msgHLayout->addWidget(gifLabel);
        adjustSize();
        listWidgetItem->setSizeHint(size());
    }
    // 其他文件
    else
    {
        msgType = FileMsg;
        FileMsgWidget *fileMsgWidget = new FileMsgWidget(file_url, this);
        ui->msgHLayout->addWidget(fileMsgWidget);

        // 设置尺寸
        adjustSize();
        listWidgetItem->setSizeHint(size());

        // 添加样式
        ui->msgWidget->setStyleSheet(
            "QWidget#msgWidget { background-color: rgb(255, 255, 255); }"
            "QWidget#msgWidget:hover { background-color: rgb(235, 235, 235); }");
    }

    // 添加文件地址属性
    setProperty("fileUrl", QVariant(file_url));

    // 设置过滤器(鼠标双击打开文件 & 鼠标移入移出变底色)
    ui->msgWidget->installEventFilter(this);
}

void ChatMsgItem::paintEvent(QPaintEvent *e)
{
    if (msgType == ImageMsg) return;

    QPainter painter(this);

    painter.setPen(Qt::transparent);

    // 设置背景色
    if (msgType == FileMsg)
    {
        if (hover)
        {
            painter.setBrush(QColor(235, 235, 235));
        }
        else
        {
            painter.setBrush(QColor(255, 255, 255));
        }
    }
    else
    {
        painter.setBrush(bubbleColor);
    }

    QPointF points[3];

    // 计算顶点坐标
    if (myMsg)
    {
        QPoint pos = ui->msgWidget->pos() + QPoint(ui->msgWidget->width(), 0);
        points[0] = pos + QPoint(5, 20);
        points[1] = pos + QPoint(-1, 15);
        points[2] = pos + QPoint(-1, 25);
    }
    else
    {
        QPoint pos = ui->msgWidget->pos();
        points[0] = pos + QPoint(-5, 20),
        points[1] = pos + QPoint(1, 15),
        points[2] = pos + QPoint(1, 25);
    }
    // 绘制三角
    painter.drawPolygon(points, 3);
}

bool ChatMsgItem::eventFilter(QObject *obj, QEvent *event)
{
    QMouseEvent *mouse_e = static_cast<QMouseEvent *>(event);
    if (obj == ui->msgWidget && event->type() == QEvent::Enter)
    {
        // 鼠标进入
        hover = true;
        update();
    }
    else if (obj == ui->msgWidget && event->type() == QEvent::Leave)
    {
        // 鼠标离开
        hover = false;
        update();
    }
    else if (obj == ui->msgWidget && event->type() == QEvent::MouseButtonPress && mouse_e->button() == Qt::RightButton && property("fileUrl").toString().mid(0, 1) != ":")
    {
        QMenu menu;
        QAction *openFile_action = menu.addAction("打开文件");
        QAction *openDir_action = menu.addAction("打开文件所在目录");
        QAction *action = menu.exec(mouse_e->globalPosition().toPoint());
        if (action == openFile_action)
        {
            openFile();
        }
        else if (action == openDir_action)
        {
            openDir();
        }
    }
    else if (obj == ui->msgWidget && event->type() == QEvent::MouseButtonDblClick && mouse_e->button() == Qt::LeftButton && property("fileUrl").toString().mid(0, 1) != ":")
    {
        // 双击文件消息可打开文件
        openFile();
    }
    return QObject::eventFilter(obj, event);
}

void ChatMsgItem::openFile()
{
    const QString &fileUrl = property("fileUrl").toString();
    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(fileUrl)))
    {
        QMessageBox::critical(this, "错误", "文件已删除");
    }
}

void ChatMsgItem::openDir()
{
    const QString &fileUrl = property("fileUrl").toString();
    QFileInfo fileInfo(fileUrl);
    QUrl folderUrl = QUrl::fromLocalFile(fileInfo.absolutePath());
    if (!QDesktopServices::openUrl(folderUrl))
    {
        QMessageBox::critical(this, "错误", "文件夹不存在");
    }
}
