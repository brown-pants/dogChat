#include "storagemanager.h"
#include <QStandardPaths>
#include <QDirIterator>
#include <QDebug>
#include <QFile>

StorageManager &StorageManager::GetInstance()
{
    static StorageManager instance;
    return instance;
}

QVector<QString> StorageManager::users() const
{
    return profileMap.keys();
}

QPixmap *StorageManager::profile(const QString user) const
{
    auto iter = profileMap.find(user);
    if (iter == profileMap.end())
    {
        return nullptr;
    }
    return iter.value();
}

StorageManager::StorageManager(QObject *parent)
    : QObject{parent}
{
    // 设置存储目录
#if defined(__unix__)
    root_dir = "~/dogChatFiles/";
#else
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    root_dir = documentsPath + "/dogChatFiles/";
#endif

    QString path = root_dir + "Users";
    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);

    // 读取所有用户
    while (it.hasNext())
    {
        QFileInfo dirInfo(it.next());
        QString user_id = dirInfo.fileName();
        QString user_path = dirInfo.filePath();

        // 查找头像图片
        QDirIterator it(user_path, QDir::Files);
        while (it.hasNext())
        {
            QFileInfo fileInfo(it.next());
            if (fileInfo.baseName() == "profile")
            {
                QString profile_path = fileInfo.absoluteFilePath();
                qDebug() << profile_path << Qt::endl;
                profileMap.insert(user_id, new QPixmap(profile_path));
            }
        }
    }
}

StorageManager::~StorageManager()
{
    for (QPixmap *profile : profileMap.values())
    {
        delete profile;
    }
}
