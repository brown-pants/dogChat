/*  ------------------------------------
 *      好友列表项类
 *  ------------------------------------
 */

#ifndef FRIENDLISTITEM_H
#define FRIENDLISTITEM_H

#include <QWidget>

namespace Ui {
class FriendListItem;
}

class FriendListItem : public QWidget
{
    Q_OBJECT

public:
    explicit FriendListItem(const QPixmap &profile, const QString &friendName, QWidget *parent = nullptr);
    ~FriendListItem();

private:
    Ui::FriendListItem *ui;
};

#endif // FRIENDLISTITEM_H
