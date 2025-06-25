#include "chooseemojiwidget.h"
#include "ui_chooseemojiwidget.h"

#include <QGraphicsDropShadowEffect>
#include <QTextBoundaryFinder>
#include <QFile>
#include <QPainter>
#include <QLabel>
#include <QMovie>

ChooseEmojiWidget::ChooseEmojiWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChooseEmojiWidget)
{
    ui->setupUi(this);

    setStyleSheet("border: none");

    // 窗口无控制栏 点击其他窗口时关闭 且 透明
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置为可选中状态
    ui->charEmojiButton->setCheckable(true);
    ui->animEmojiButton->setCheckable(true);
    ui->charEmojiButton->setChecked(true);

    // 设置外边框阴影参数
    int shadowWidth = 20;
    layout()->setContentsMargins(shadowWidth, shadowWidth, shadowWidth, shadowWidth);

    //设置外边框阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(120, 120, 120));
    shadow->setBlurRadius(shadowWidth);
    ui->shadowWidget->setGraphicsEffect(shadow);

    // 加载全部表情
    loadCharEmojis();
    loadAnimEmojis();
}

ChooseEmojiWidget::~ChooseEmojiWidget()
{
    delete ui;
}

void ChooseEmojiWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    painter.setPen(Qt::transparent);
    painter.setBrush(QColor(255, 255, 255, 255));

    QPointF points[3];

    QPoint pos = QPoint(width() / 2, height() - layout()->contentsMargins().bottom());
    points[0] = pos + QPoint(0, 18);
    points[1] = pos + QPoint(20, -2);
    points[2] = pos + QPoint(-20, -2);

    // 绘制三角
    painter.drawPolygon(points, 3);
}

void ChooseEmojiWidget::loadCharEmojis()
{
    const int tableColumn = 10;
    const int buttonSize = 30;

    // 设置表格
    ui->charEmojiTableWidget->setShowGrid(false);
    ui->charEmojiTableWidget->verticalHeader()->setVisible(false);
    ui->charEmojiTableWidget->horizontalHeader()->setVisible(false);
    ui->charEmojiTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->charEmojiTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->charEmojiTableWidget->setColumnCount(tableColumn);
    ui->charEmojiTableWidget->insertRow(0);

    // 设置窗口大小
    setFixedSize(tableColumn * buttonSize +
                 ui->shadowWidget->layout()->contentsMargins().left() + ui->shadowWidget->layout()->contentsMargins().right() +
                 layout()->contentsMargins().left() + layout()->contentsMargins().right() +
                 10, 500);

    // 从文件读取emoji序列
    QFile file(":/res/emoji/emoji.txt");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "emoji加载失败" << Qt::endl;
        return;
    }
    QString input = file.readAll();

    // 将读取到的所有emoji顺序放入表格中
    QString emoji;
    int row = 0;
    int col = 0;
    do
    {
        // 跳过换行符和空格
        if (input[0] == '\n' || input[0] == '\r' || input[0] == ' ')
        {
            input.remove(0, 1);
            continue;
        }

        // 查找下一个emoji
        QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, input);
        if (finder.toNextBoundary() > 0)
        {
            emoji = input.left(finder.position());
            input = input.mid(finder.position(), input.length());
        }

        // 将emoji显示在表格中
        QPushButton *button = new QPushButton(this);
        QFont font;
        font.setPointSize(15);
        button->setFont(font);
        button->setText(emoji);
        button->setFixedSize(buttonSize, buttonSize);
        if (col >= tableColumn)
        {
            row ++;
            col = 0;
            ui->charEmojiTableWidget->insertRow(ui->charEmojiTableWidget->rowCount());

        }
        ui->charEmojiTableWidget->setCellWidget(row, col, button);
        ui->charEmojiTableWidget->setColumnWidth(col, button->width());
        ui->charEmojiTableWidget->setRowHeight(row, button->height());
        col ++;

        // 处理按钮点击信号
        connect(button, &QPushButton::clicked, [emoji, this](){
            emit charEmojiClicked(emoji);
            this->hide();
        });
    } while(input.length());
}

void ChooseEmojiWidget::loadAnimEmojis()
{
    const int tableColumn = 5;
    int buttonSize = width() -
                     ui->shadowWidget->layout()->contentsMargins().left() - ui->shadowWidget->layout()->contentsMargins().right() -
                     layout()->contentsMargins().left() - layout()->contentsMargins().right();
    buttonSize /= tableColumn;

    // 设置表格
    ui->animEmojiTableWidget->setShowGrid(false);
    ui->animEmojiTableWidget->verticalHeader()->setVisible(false);
    ui->animEmojiTableWidget->horizontalHeader()->setVisible(false);
    ui->animEmojiTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->animEmojiTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->animEmojiTableWidget->setColumnCount(tableColumn);
    ui->animEmojiTableWidget->insertRow(0);

    // 获取动图url
    QVector<QString> gifUrlArray;
    for (int i = 1; i <= 11; i ++)
    {
        gifUrlArray.push_back(QString(":/res/gif/anim%1.gif").arg(i));
    }

    int row = 0;
    int col = 0;

    for (const QString &url : gifUrlArray)
    {
        // 创建按钮
        QPushButton *button = new QPushButton(this);
        button->setFixedSize(buttonSize, buttonSize);

        // 加载动画
        QLabel *gifLabel = new QLabel(button);
        QMovie *movie = new QMovie(url, QByteArray(), this);

        // 预先设置缩放尺寸
        movie->setScaledSize(QSize(buttonSize, buttonSize));

        // 缓存所有帧提高性能
        movie->setCacheMode(QMovie::CacheAll);

        // 设置标签大小和位置
        gifLabel->setFixedSize(button->size());

        // 使用高质量缩放但不中断动画
        connect(movie, &QMovie::frameChanged, [=]{
            gifLabel->setPixmap(movie->currentPixmap().scaled(
                buttonSize,
                buttonSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            ));
        });

        // 将标签放在按钮上
        gifLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // 使点击穿透到按钮
        gifLabel->raise(); // 确保在最上层

        // 开始播放动画
        movie->start();

        // 放入表格
        if (col >= tableColumn)
        {
            row ++;
            col = 0;
            ui->animEmojiTableWidget->insertRow(ui->animEmojiTableWidget->rowCount());
        }
        ui->animEmojiTableWidget->setCellWidget(row, col, button);
        ui->animEmojiTableWidget->setColumnWidth(col, button->width());
        ui->animEmojiTableWidget->setRowHeight(row, button->height());
        col ++;

        // 处理按钮点击信号
        connect(button, &QPushButton::clicked, [url, this](){
            emit animEmojiClicked(url);
            this->hide();
        });
    }
}

void ChooseEmojiWidget::on_charEmojiButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->charEmojiWidget);
    ui->charEmojiButton->setChecked(true);
    ui->animEmojiButton->setChecked(false);
}


void ChooseEmojiWidget::on_animEmojiButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->animEmojiWidget);
    ui->animEmojiButton->setChecked(true);
    ui->charEmojiButton->setChecked(false);
}

