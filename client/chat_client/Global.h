#ifndef GLOBAL_H
#define GLOBAL_H
#include <QString>
#include <QPIxmap>
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
    ChatMsgInfo(const QString &time, const QString &type, const QString &msg, const QString &user, bool self)
        : time(time), type(type), msg(msg), user(user), self(self) {}
    QString time;
    QString type;
    QString msg;
    QString user;
    bool self;
};

#endif // GLOBAL_H
