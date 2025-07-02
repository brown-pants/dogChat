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

StorageManager::StorageManager(QObject *parent)
    : QObject{parent}
{
    // 设置存储目录
#if defined(__unix__)
    storageDir = "~/dogChatFiles/";
#else
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + '/';
    storageDir = documentsPath;
#endif

    QString path = storageDir + "Users";
    QDirIterator it(path, QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        if (fileInfo.isDir() && filePath != path)
        {

        }
    }
}
