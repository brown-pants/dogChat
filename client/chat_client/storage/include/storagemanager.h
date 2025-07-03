/*  ------------------------------------
 *      存储管理类
 *  存储结构:
 *  root_dir/
 *      config.json
 *      Users/
 *          user_id/
 *              profile.jpg
 *              msg/
 *                  friend_id.json
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

    QVector<QString> users() const;
    QPixmap *profile(const QString user) const;

    QString serverIp() const;
    QString serverPort() const;

private:
    explicit StorageManager(QObject *parent = nullptr);
    ~StorageManager();
    void saveConfig();

    QString root_dir;
    QMap<QString, QPixmap *> profileMap;
    QJsonObject configJson;
};

#endif // STORAGEMANAGER_H
