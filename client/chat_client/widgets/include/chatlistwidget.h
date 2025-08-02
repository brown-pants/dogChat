/*  ------------------
 *      聊天列表类
 *  ------------------
 */

#ifndef CHATLISTWIDGET_H
#define CHATLISTWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include "chatlistitem.h"

namespace Ui {
class ChatListWidget;
}

class ChatListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatListWidget(QWidget *parent = nullptr);
    ~ChatListWidget();
    int removeChatItem(const QString &friendName);
    ChatListItem *getItemWidget(const QString &user);
    void setSelect(const QString &user);

signals:
    void sig_insertChatItem(const QPixmap &profile, const QString &friendName, const QString &msg, const QString &time, int row = -1);
    void selectChat(const QString &user);

public slots:
    void insertChatItem(const QPixmap &profile, const QString &friendName, const QString &msg, const QString &time, int row = -1);

private:
    Ui::ChatListWidget *ui;
    QMap<QString, QListWidgetItem *> items_map;
};

#endif // CHATLISTWIDGET_H
