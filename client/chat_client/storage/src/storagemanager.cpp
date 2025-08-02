#include "storagemanager.h"
#include "util.h"
#include <QStandardPaths>
#include <QDirIterator>
#include <QDebug>
#include <QFile>
#include <QtConcurrent/QtConcurrent>

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

void StorageManager::addUser(const QString &user, const QByteArray &profile)
{
    QDir dir = root_dir + "Users/" + user;
    QString url = root_dir + "Users/" + user + "/profile";
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    Util::WriteToFile(url, profile);
}

QString StorageManager::serverIp() const
{
    return configJson["server"].toObject()["ip"].toString();
}

QString StorageManager::serverPort() const
{
    return configJson["server"].toObject()["port"].toString();
}

void StorageManager::setServerIp(const QString &ip)
{
    QJsonObject serverObj = configJson["server"].toObject();
    serverObj["ip"] = ip;
    configJson["server"] = serverObj;
}

void StorageManager::setServerPort(const QString &port)
{
    QJsonObject serverObj = configJson["server"].toObject();
    serverObj["port"] = port;
    configJson["server"] = serverObj;
}

void StorageManager::insertToChatList(const QString &user, int row)
{
    chatListJson.insert(row, user);
}

void StorageManager::removeFromChatList(int row)
{
    chatListJson.removeAt(row);
}

QVector<QString> StorageManager::chatList() const
{
    QVector<QString> chatListVec;
    for (auto v : chatListJson)
    {
        Util::JsonValueToString(v, [&chatListVec](const QString &user){
            chatListVec.push_back(user);
        });
    }
    return chatListVec;
}

void StorageManager::loadChatList(const QString &user)
{
    // 获取聊天列表
    QString path = root_dir + "Users/" + user + "/ChatList.json";
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
        Util::JsonDocToArray(jsonDoc, [this](QJsonArray arr){
            chatListJson = arr;
        });
        file.close();
    }
}

void StorageManager::saveChatList(const QString &user)
{
    QString chatList_path = root_dir + "Users/" + user + "/ChatList.json";
    QFile chatList_file(chatList_path);
    if (!chatList_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "聊天列表文件打开失败:" << chatList_path <<  " error: " << chatList_file.errorString() << Qt::endl;
    }
    else
    {
        QJsonDocument doc = QJsonDocument(chatListJson);
        chatList_file.write(doc.toJson());
        chatList_file.close();
    }
}

void StorageManager::loadChatMsg(const QString &cur_user, const QString &userB)
{
    // 获取聊天消息
    if (chatMsg.find(userB) != chatMsg.end())
    {
        return;
    }

    QDir dir(root_dir + "Users/" + cur_user + "/ChatMsg/");
    if (!dir.exists())
    {
        dir.mkdir(".");
    }

    QString path = dir.path() + "/" + userB + ".json";
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        QJsonArray msg;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
        Util::JsonDocToArray(jsonDoc, [this, &msg](QJsonArray arr){
            msg = arr;
        });
        file.close();
        chatMsg[userB] = msg;
    }
    else
    {
        qDebug() << path << " 不存在" << Qt::endl;
        chatMsg[userB] = QJsonArray();
    }
}

ChatMsgInfo StorageManager::getChatMsg(const QString &user, int row)
{
    ChatMsgInfo msg;
    Util::JsonValueToObj(chatMsg[user][row], [&msg](QJsonObject obj){
        msg.time = obj["time"].toString();
        msg.type = obj["type"].toString();
        msg.msg = obj["msg"].toString();
        msg.user = obj["user"].toString();
        msg.self = obj["self"].toBool();
    });
    return msg;
}

void StorageManager::addChatMsg(const QString &user, ChatMsgInfo info)
{
    QJsonObject json;
    json["time"] = info.time;
    json["type"] = info.type;
    json["msg"] = info.msg;
    json["user"] = info.user;
    json["self"] = info.self;
    chatMsg[user].push_front(json);
}

int StorageManager::chatMsgCount(const QString &user)
{
    return chatMsg[user].count();
}

void StorageManager::saveChatMsg(const QString &curUser, const QString &user)
{
    QString chatMsg_path = root_dir + "Users/" + curUser + "/ChatMsg/" + user + ".json";
    QFile chatMsg_file(chatMsg_path);
    if (!chatMsg_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "聊天列表文件打开失败:" << chatMsg_path <<  " error: " << chatMsg_file.errorString() << Qt::endl;
    }
    else
    {
        QJsonDocument doc = QJsonDocument(chatMsg[user]);
        chatMsg_file.write(doc.toJson());
        chatMsg_file.close();
    }
}

QString StorageManager::saveFile(const QString &userA, const QString &userB, const QString &fileName, const QByteArray &data)
{
    qDebug() << "saveFile thread: " << QThread::currentThread();
    QString dirPath = root_dir + "Users/" + userA + "/Files/" + userB;
    QString fileUrl = dirPath + "/" + fileName;

    QDir dir(dirPath);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }

    QFileInfo fileInfo(fileUrl);
    QString suffix = fileInfo.suffix();
    QString baseName = fileInfo.completeBaseName();
    int counter = 0;

    // 循环直到找到不重复的文件名
    while (QFileInfo::exists(fileUrl))
    {
        fileUrl = QString("%1/%2(%3).%4").arg(dirPath).arg(baseName).arg(counter).arg(suffix);
        counter++;
    }
    Util::WriteToFile(fileUrl, data);

    qDebug() << "saveFileEnd" << Qt::endl;

    return fileUrl;
}

StorageManager::StorageManager()
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
        QString user_name = dirInfo.fileName();
        QString user_path = dirInfo.filePath();
        profileMap.insert(user_name, new QPixmap(user_path + "/profile"));
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
