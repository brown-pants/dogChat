/*  ------------------------------------
 *      文件消息类
 *  ------------------------------------
 */

#ifndef FILEMSGWIDGET_H
#define FILEMSGWIDGET_H

#include <QWidget>

namespace Ui {
class FileMsgWidget;
}

class FileMsgWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileMsgWidget(const QString &fileUrl, QWidget *parent = nullptr);
    ~FileMsgWidget();

private:
    Ui::FileMsgWidget *ui;
};

#endif // FILEMSGWIDGET_H
