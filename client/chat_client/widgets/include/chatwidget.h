/*  ------------------
 *      聊天界面类
 *  ------------------
 */

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QDateTime>

#include "chooseemojiwidget.h"
#include "global.h"
#include "chatmsgitem.h"

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();
    void loadMessages(const QString &user);
    QString curUser() const;
    void appendMsg(ChatMsgInfo msg);

signals:
    void sendMsg(const QString &user, ChatMsgInfo msg);
    void sendFail(const QString &user, const QString &msg_id);

private slots:
    void on_sendButton_clicked();

    void on_emojiButton_clicked();

    void on_imgButton_clicked();

    void on_fileButton_clicked();

private:
    Ui::ChatWidget *ui;
    ChooseEmojiWidget *chooseEmojiWidget;
    QString m_curUser;
    QDateTime lastMsgTime;
    QDateTime _LastMsgTime;
    int msgRow;
    QMap<QString, ChatMsgItem *> m_msgItems;

    ChatMsgItem *addTextMsg(bool myMsg, const QPixmap &profile, const QString &text, int row = -1);
    ChatMsgItem *addFileMsg(bool myMsg, const QPixmap &profile, const QString &url, int row = -1);
    void addTime(const QString &time, int row = -1);
    void addLoadOld(int row);
    void loadMsg(const QString &user);
    void clearMsg();
    void sendFileMsg(const QString &url);
};

#endif // CHATWIDGET_H
