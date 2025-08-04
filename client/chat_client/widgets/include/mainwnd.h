/*  ------------------
 *      主窗体类
 *  ------------------
 */

#ifndef MAINWND_H
#define MAINWND_H

#include <QWidget>
#include "chatwidget.h"
#include "chatlistwidget.h"
#include "friendlistwidget.h"
#include "newfriendswidget.h"
#include "finduserwidget.h"
#include "friendinfowidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWnd;
}
QT_END_NAMESPACE

class MainWnd : public QWidget
{
    Q_OBJECT

    friend class MainWndLoader;
public:
    static MainWnd *GetInstance();

    MainWnd(QWidget *parent = nullptr);
    ~MainWnd();

    QString curUser() const;
    QPixmap curUserProfile() const;
    QPixmap getUserProfile(const QString &user) const;

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:
    void on_chatButton_clicked();

    void on_friendButtom_clicked();

    void on_profileButton_clicked();

    void RecvMsg(const QString &user, const QString &time, const QString &msg, bool file_msg);

    void loadMsg(const QString &user, ChatMsgInfo msg);

    void toChat(const QString &user);

private:
    Ui::MainWnd *ui;

    static MainWnd *__instance;

    QString curr_user;
    QPixmap user_profile;
    ChatWidget *m_ChatWidget;
    ChatListWidget *m_ChatListWidget;
    FriendListWidget *m_FriendListWidget;
    NewFriendsWidget *m_NewFriendsWidget;
    FriendInfoWidget *m_FriendInfoWidget;
    FindUserWidget *m_userWidget;
    QWidget *p_curChatWidget;
    QWidget *p_curFriendWidget;

    QMutex mutex;

signals:
    void sig_loadMsg(const QString &user, ChatMsgInfo msgInfo);
};


class MainWndLoader : public QObject
{
    Q_OBJECT
public:
    MainWndLoader(QObject *parent = nullptr);
    ~MainWndLoader();

public slots:
    void load(const QString &user, const QPixmap &profile);
    void load_Friends(const QVector<FriendInfo> &friend_arr);
    void load_applyFriends(const QVector<ApplyFriendInfo> &apply_arr);
    void load_chatList();

private:
    MainWnd *mainwnd;

signals:
    void finished();
};

#endif // MAINWND_H
