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

class StorageManager : public QObject
{
    Q_OBJECT
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

private:
    explicit StorageManager(QObject *parent = nullptr);
    ~StorageManager();

    QString root_dir;
    QMap<QString, QPixmap *> profileMap;
    QJsonObject configJson;
};

#endif // STORAGEMANAGER_H
