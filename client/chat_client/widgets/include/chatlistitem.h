/*  ------------------------------------
 *      聊天列表项类
 *  ------------------------------------
 */

#ifndef CHATLISTITEM_H
#define CHATLISTITEM_H

#include <QWidget>

namespace Ui {
class ChatListItem;
}

class ChatListItem : public QWidget
{
    Q_OBJECT

public:
    explicit ChatListItem(const QPixmap &profile, const QString &friendName, const QString &msg, const QString &time, QWidget *parent = nullptr);
    ~ChatListItem();

    void hideRedCircle();
    void showRedCircle();

protected:
    void resizeEvent(QResizeEvent *e);

private:
    Ui::ChatListItem *ui;
    QString msgText;
};

#endif // CHATLISTITEM_H
