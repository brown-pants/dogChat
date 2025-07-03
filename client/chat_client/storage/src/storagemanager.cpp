#include "storagemanager.h"
#include "util.h"
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

QString StorageManager::serverIp() const
{
    return configJson["server"].toObject()["ip"].toString();
}

QString StorageManager::serverPort() const
{
    return configJson["server"].toObject()["port"].toString();
}

StorageManager::StorageManager(QObject *parent)
    : QObject{parent}
{
    // 设置存储目录
#if defined(__unix__)
    root_dir = Util::GetParentDirectory(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)) + "/dogChatFiles/";
#else
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    root_dir = documentsPath + "/dogChatFiles/";
#endif

    qDebug() << "存储路径: " << root_dir << Qt::endl;

    // 创建存储文件夹
    QDir dir(root_dir);
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            qDebug() << "存储文件夹创建失败:" << root_dir << Qt::endl;
            return;
        }
        else qDebug() << "存储文件夹创建成功:" << root_dir << Qt::endl;
    }

    // 读取所有用户
    QString path = root_dir + "Users";
    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);

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

    // 获取服务器ip和端口
    QString config_path = root_dir + "config.json";
    QFile config_file(config_path);
    QJsonDocument jsonDoc;
    // 读取文件
    if(config_file.open(QIODevice::ReadOnly))
    {
        jsonDoc = QJsonDocument::fromJson(config_file.readAll());
        config_file.close();
    }
    else
    {
        jsonDoc = QJsonDocument::fromJson("");
    }
    QString serverIp, serverPort;
    // 解析json
    Util::JsonDocToObj(jsonDoc, [&](QJsonObject rootObj){
        Util::JsonValueToObj(rootObj["server"], [&](QJsonObject serverObj){
            Util::JsonValueToString(serverObj["ip"], [&](QString ip){
                serverIp = ip;
            });
            Util::JsonValueToString(serverObj["port"], [&](QString port){
                serverPort = port;
            });
        });
    });
    // 设置服务器ip和端口
    if (serverIp.isNull())
    {
        serverIp = "127.0.0.1";
    }
    if (serverPort.isNull())
    {
        serverPort = "8080";
    }
    // 写入Json
    QJsonObject serverObj;
    serverObj["ip"] = serverIp;
    serverObj["port"] = serverPort;
    configJson["server"] = serverObj;
    QJsonDocument doc = QJsonDocument(configJson);
    // 写入文件
    saveConfig();
}

StorageManager::~StorageManager()
{
    for (QPixmap *profile : profileMap.values())
    {
        delete profile;
    }
}

void StorageManager::saveConfig()
{
    QString config_path = root_dir + "config.json";
    QFile config_file(config_path);
    if (!config_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "配置文件打开失败:" << config_path <<  " error: " << config_file.errorString() << Qt::endl;
    }
    else
    {
        QJsonDocument doc = QJsonDocument(configJson);
        config_file.write(doc.toJson());
        config_file.close();
    }
}
