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

protected:
    void resizeEvent(QResizeEvent *e);

private:
    Ui::FriendListItem *ui;
    QString friendNameText;
};

#endif // FRIENDLISTITEM_H
