#include "filemsgwidget.h"
#include "ui_filemsgwidget.h"

#include <QFileIconProvider>
#include <QIcon>

FileMsgWidget::FileMsgWidget(const QString &fileUrl, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileMsgWidget)
{
    ui->setupUi(this);

    // 获取文件信息
    QFileIconProvider iconProvider;
    QFileInfo fileInfo(fileUrl);

    // 设置图标和文件名
    ui->fileIconLabel->setPixmap(iconProvider.icon(fileInfo).pixmap(35, 40));
    ui->fileIconLabel->setScaledContents(true);
    ui->fileIconLabel->setFixedSize(35, 40);
    ui->fileIconWidget->setFixedWidth(35);
    ui->fileNameLabel->setText(fileInfo.fileName());

    // 计算文件大小
    float fileSize = fileInfo.size();
    int cnt = 0;

    while (cnt++ < 3 && fileSize / 1024 >= 1.0f)
    {
        fileSize /= 1024;
    }

    ui->fileSizeLabel->setText(QString::number(fileSize) + (cnt == 1 ? "B" : cnt == 2 ? "KB" : cnt == 3 ? "MB" : "GB"));
    this->setFixedSize(200, 70);
}

FileMsgWidget::~FileMsgWidget()
{
    delete ui;
}
