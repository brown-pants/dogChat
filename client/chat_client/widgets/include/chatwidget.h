/*  ------------------
 *      聊天界面类
 *  ------------------
 */

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>

#include "chooseemojiwidget.h"

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();

private slots:
    void on_sendButton_clicked();

    void on_emojiButton_clicked();

    void on_imgButton_clicked();

    void on_fileButton_clicked();

private:
    Ui::ChatWidget *ui;
    ChooseEmojiWidget *chooseEmojiWidget;

    void addTextMsg(bool myMsg, const QPixmap &profile, const QString &text);
    void addFileMsg(bool myMsg, const QPixmap &profile, const QString &url);
};

#endif // CHATWIDGET_H
