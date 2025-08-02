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
    explicit FriendListItem(const QPixmap &profile, const QString &friendName, bool find = false, QWidget *parent = nullptr);
    ~FriendListItem();
    void setText(const QString &text);
    QString getUserName();
    QPixmap getUserProfile();

protected:
    void resizeEvent(QResizeEvent *e);

private:
    Ui::FriendListItem *ui;
    QString friendNameText;
    QPixmap profile;
    bool m_find;
};

#endif // FRIENDLISTITEM_H
