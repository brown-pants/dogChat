#ifndef GLOBAL_H
#define GLOBAL_H
#include <QString>
#include <QPixmap>
#include "util.h"

struct ApplyFriendInfo
{
    QString user_apply;
    QString leave_msg;
    QString apply_state;
    QString profile;
};

struct FriendInfo
{
    FriendInfo(const QString &name = "", const QPixmap &profile = QPixmap()) : name(name), profile(profile) {}
    bool operator<(const FriendInfo &other) const
    {
        return Util::Pinyin(name) < Util::Pinyin(other.name);
    }
    QString name;
    QPixmap profile;
};

struct ChatMsgInfo
{
    ChatMsgInfo() {}
    ChatMsgInfo(const QString &id, const QString &time, const QString &type, const QString &msg, const QString &user, bool send_succ, bool self)
        : id(id), time(time), type(type), msg(msg), user(user), send_succ(send_succ), self(self) {}
    QString id;
    QString time;
    QString type;
    QString msg;
    QString user;
    bool send_succ;
    bool self;
};

#endif // GLOBAL_H
