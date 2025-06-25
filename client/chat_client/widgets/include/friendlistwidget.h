/*  ------------------------------------
 *      好友列表类
 *  ------------------------------------
 */

#ifndef FRIENDLISTWIDGET_H
#define FRIENDLISTWIDGET_H

#include <QWidget>
#include <set>
#include "util.h"

namespace Ui {
class FriendListWidget;
}

class FriendListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FriendListWidget(QWidget *parent = nullptr);
    ~FriendListWidget();

    struct FriendInfo
    {
        FriendInfo(const QString &id, const QString &name, const QPixmap &profile) : id(id), name(name), profile(profile) {}
        bool operator<(const FriendInfo &other) const
        {
            return Util::Pinyin(name) < Util::Pinyin(other.name);
        }
        QString id;
        QString name;
        QPixmap profile;
    };

private:
    Ui::FriendListWidget *ui;

    void insertLabelItem(const QString &text, int row = -1);
    void insertFriendItem(const QPixmap &profile, const QString &friendName, int row = -1);
    void loadFriends(const std::multiset<FriendInfo> &friends);
};

#endif // FRIENDLISTWIDGET_H
