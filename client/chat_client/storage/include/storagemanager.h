#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QMap>

class StorageManager : public QObject
{
    Q_OBJECT
public:
    static StorageManager &GetInstance();

private:
    explicit StorageManager(QObject *parent = nullptr);
    QString storageDir;
    QMap<QString, QPixmap> users_profile;
};

#endif // STORAGEMANAGER_H
