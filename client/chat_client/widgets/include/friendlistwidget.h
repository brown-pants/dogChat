/*  ------------------------------------
 *      好友列表类
 *  ------------------------------------
 */

#ifndef FRIENDLISTWIDGET_H
#define FRIENDLISTWIDGET_H

#include <QWidget>
#include <set>
#include <QMutex>
#include "global.h"
#include "finduserwidget.h"

namespace Ui {
class FriendListWidget;
}

class FriendListWidget : public QWidget
{
    Q_OBJECT
    friend class MainWnd;
public:
    explicit FriendListWidget(QWidget *parent = nullptr);
    ~FriendListWidget();
    void loadFriends(const std::set<FriendInfo> &friends);
    QPixmap getFriendProfile(const QString &user) const;

signals:
    void newFriendItemClicked();
    void sig_insertLabelItem(const QString &text, int row = -1);
    void sig_insertFriendItem(const QPixmap &profile, const QString &friendName, bool find = false, int row = -1);
    void itemClicked(const QString &user, const QPixmap &profile);

public slots:
    void on_addFriendButton_clicked();
    void on_cancelButton_clicked();
    void slo_findFriend(const QString &status, const QString &profile, const QString &isFriend);
    void addFriend(const QString &user, const QString &profile);
    void removeFriend(const QString &user);
    void insertLabelItem(const QString &text, int row = -1);
    void insertFriendItem(const QPixmap &profile, const QString &friendName, bool find = false, int row = -1);

private:
    Ui::FriendListWidget *ui;
    FindUserWidget *m_FindUserWidget;
    int m_state;
    QMutex m_mtx;
    QMap<QString, QPixmap> profiles;
};

#endif // FRIENDLISTWIDGET_H
