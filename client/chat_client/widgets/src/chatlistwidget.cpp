#include "chatlistwidget.h"
#include "ui_chatlistwidget.h"
#include "chatlistitem.h"

#include <QScrollBar>

ChatListWidget::ChatListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatListWidget)
{
    ui->setupUi(this);

    // 设置搜索框图标
    QAction *searchAction = new QAction(ui->searchLineEdit);
    searchAction->setIcon(QIcon(":/res/img/Search.png"));
    ui->searchLineEdit->addAction(searchAction, QLineEdit::LeadingPosition);

    // 设置列表滑动条滚动速度
    ui->listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listWidget->verticalScrollBar()->setSingleStep(10);

    for(int i = 0; i < 100; i ++)
    {
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(QSize(10, 65));
        ui->listWidget->addItem(item);

        ChatListItem *pItemWidget = new ChatListItem(QPixmap("D:\\code\\MyChat\\xjm.gif"), "小鸡毛", "你好呀，我是小鸡毛", "2025/06/16", ui->listWidget);
        item->setData(Qt::UserRole, QVariant::fromValue(pItemWidget));

        ui->listWidget->setItemWidget(item, pItemWidget);
    }
    connect(ui->listWidget, &QListWidget::itemClicked, [](QListWidgetItem *item){
        ChatListItem *pItemWidget = item->data(Qt::UserRole).value<ChatListItem *>();
        pItemWidget->hideRedCircle();
    });
}

ChatListWidget::~ChatListWidget()
{
    delete ui;
}
