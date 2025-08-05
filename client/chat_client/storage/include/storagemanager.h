/*  ------------------------------------
 *      存储管理类
 *  存储结构:
 *  root_dir/
 *      config.json
 *      Users/
 *          user_id/
 *              profile.jpg
 *          *
 *  ------------------------------------
 */

#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include "global.h"

class StorageManager
{
public:
    static StorageManager &GetInstance();

    void saveConfig();

    QVector<QString> users() const;
    QPixmap *profile(const QString user) const;
    void addUser(const QString &user, const QByteArray &profile);

    QString serverIp() const;
    QString serverPort() const;

    void setServerIp(const QString &ip);
    void setServerPort(const QString &port);

    void insertToChatList(const QString &user, int row);
    void removeFromChatList(int row);
    QVector<QString> chatList() const;
    void loadChatList(const QString &user);
    void saveChatList(const QString &user);

    void loadChatMsg(const QString &cur_user, const QString &userB);
    ChatMsgInfo getChatMsg(const QString &user, int row);
    void addChatMsg(const QString &user, ChatMsgInfo info);
    void removeChatMsg(const QString &user, const QString &id);
    int chatMsgCount(const QString &user);
    void saveChatMsg(const QString &curUser, const QString &user);
    void setChatMsgFail(const QString &user, const QString &msg_id);

    QString saveFile(const QString &userA, const QString &userB, const QString &fileName, const QByteArray &data);

private:
    explicit StorageManager();
    ~StorageManager();

    QString root_dir;
    QMap<QString, QPixmap *> profileMap;
    QJsonObject configJson;
    QJsonArray chatListJson;

    QMap<QString, QJsonArray> chatMsg;
};

#endif // STORAGEMANAGER_H
