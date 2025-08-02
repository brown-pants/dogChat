#include "tcpclient.h"
#include "storagemanager.h"
#include "util.h"
#include <QJsonDocument>
#include <QJsonObject>

TcpClient::TcpClient()
    : _b_recv_pending(false)
{
    _thread = new QThread;
    this->moveToThread(_thread);

    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);

    // 等待初始化完成再向下执行
    init_mutex.lock();
    _thread->start();
    init_condition.wait(&init_mutex);
    init_mutex.unlock();
}

TcpClient::~TcpClient()
{
    qDebug() << "TcpClient Destructor called in thread:" << QThread::currentThread() << Qt::endl;
    _thread->quit();
    _thread->wait();
    delete _thread;
}

QByteArray TcpClient::toTcpData(QByteArray data)
{
    QByteArray tcp_data;
    QDataStream stream(&tcp_data, QDataStream::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian); // 网络字节序
    stream << (quint32)data.length();
    stream.writeRawData(data, data.length());
    return tcp_data;
}

void TcpClient::sendJson(const QJsonObject &json)
{
    QByteArray jsonByte = QJsonDocument(json).toJson(QJsonDocument::Compact);
    QByteArray data = toTcpData(jsonByte);
    _socket->write(data);
}

void TcpClient::handleMsg(const QByteArray &body)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(body);
    Util::JsonDocToObj(jsonDoc, [&](QJsonObject obj){
        Util::JsonValueToString(obj["type"], [&](const QString &type){
            if (type == "Regist")
            {
                Util::JsonValueToString(obj["status"], [&](const QString &status){
                    emit sig_regist_over(status);
                });
            }
            else if (type == "Login")
            {
                QString _status, _profile;
                Util::JsonValueToString(obj["status"], [&](const QString &status){
                    _status = status;
                });
                Util::JsonValueToString(obj["profile"], [&](const QString &profile){
                    _profile = profile;
                });
                emit sig_login_over(_status, _profile);
            }
            else if (type == "FindUser")
            {
                QString _status, _profile, _isFriend;
                Util::JsonValueToString(obj["status"], [&](const QString &status){
                    _status = status;
                });
                Util::JsonValueToString(obj["profile"], [&](const QString &profile){
                    _profile = profile;
                });
                Util::JsonValueToString(obj["isFriend"], [&](const QString &isFriend){
                    _isFriend = isFriend;
                });
                emit sig_findUser_over(_status, _profile, _isFriend);
            }
            else if (type == "ApplyFriendResponse")
            {
                Util::JsonValueToString(obj["status"], [&](const QString &status){
                    emit sig_applyFriend_over(status);
                });
            }
            else if (type == "RecvApplyFriendRequest")
            {
                Util::JsonValueToArray(obj["apply_arr"], [&](const QJsonArray &array){
                    QVector<ApplyFriendInfo> apply_arr;
                    for (auto item : array)
                    {
                        Util::JsonValueToObj(item, [&](QJsonObject apply){
                            ApplyFriendInfo info;
                            Util::JsonValueToString(apply["user_apply"], [&](const QString &user_apply){
                                info.user_apply = user_apply;
                            });
                            Util::JsonValueToString(apply["leave_msg"], [&](const QString &leave_msg){
                                info.leave_msg = leave_msg;
                            });
                            Util::JsonValueToString(apply["apply_state"], [&](const QString &apply_state){
                                info.apply_state = apply_state;
                            });
                            Util::JsonValueToString(apply["profile"], [&](const QString &profile){
                                info.profile = profile;
                            });
                            apply_arr.push_back(info);
                        });
                    }
                    emit sig_recvApplyFriendRequest_over(apply_arr);
                });
            }
            else if (type == "RecvFriends")
            {
                Util::JsonValueToArray(obj["friend_arr"], [&](const QJsonArray &array){
                    QVector<FriendInfo> friend_arr;
                    for (auto item : array)
                    {
                        Util::JsonValueToObj(item, [&](QJsonObject apply){
                            FriendInfo info;
                            Util::JsonValueToString(apply["user"], [&](const QString &user){
                                info.name = user;
                            });
                            Util::JsonValueToString(apply["profile"], [&](const QString &profile){
                                QPixmap profilePixmap;
                                profilePixmap.loadFromData(QByteArray::fromBase64(profile.toUtf8()));
                                info.profile = profilePixmap;
                            });
                            friend_arr.push_back(info);
                        });
                    }
                    emit sig_recvFriends_over(friend_arr);
                });
            }
            else if (type == "AddedFriend")
            {
                QString _user, _profile;
                Util::JsonValueToString(obj["user"], [&](const QString &user){
                    _user = user;
                });
                Util::JsonValueToString(obj["profile"], [&](const QString &profile){
                    _profile = profile;
                });
                emit sig_addedFriend_over(_user, _profile);
            }
            else if (type == "ApplyFriendRequest")
            {
                QString _user, _leave, _profile;
                Util::JsonValueToString(obj["user"], [&](const QString &user){
                    _user = user;
                });
                Util::JsonValueToString(obj["leave"], [&](const QString &leave){
                    _leave = leave;
                });
                Util::JsonValueToString(obj["profile"], [&](const QString &profile){
                    _profile = profile;
                });
                emit sig_applyFriendRequest(_user, _leave, _profile);
            }
            else if (type == "RemoveFriend")
            {
                QString _user;
                Util::JsonValueToString(obj["user"], [&](const QString &user){
                    _user = user;
                });
                emit sig_removeFriend_over(_user);
            }
            else if (type == "RecvMsg")
            {
                qDebug() << "RecvMsg begin" << Qt::endl;
                QString _user, _time, _msg;
                bool _file;
                Util::JsonValueToString(obj["user"], [&](const QString &user){
                    _user = user;
                });
                Util::JsonValueToString(obj["time"], [&](const QString &time){
                    _time = time;
                });
                Util::JsonValueToString(obj["msg"], [&](const QString &msg){
                    _msg = msg;
                });
                _file = obj["file_msg"].toBool();
                emit sig_recvMsg(_user, _time, _msg, _file);
                qDebug() << "RecvMsg end" << Qt::endl;
            }
            else if (type == "Offline")
            {
                emit sig_offline();
            }
        });
    });
}

void TcpClient::init()
{
    qDebug() << "TcpClient Init..." << Qt::endl;
    _socket = new QTcpSocket(this);
    connect(this, &TcpClient::sig_tcp_connect, this, &TcpClient::slo_tcp_connect);
    connect(this, &TcpClient::sig_tcp_disconnect, this, &TcpClient::slo_tcp_disconnect);

    connect(_socket, &QTcpSocket::connected, [&]() {
        qDebug() << "Connected to server!";
        emit connected();
    });

    connect(_socket, &QTcpSocket::disconnected, [&]() {
        qDebug() << "Disconnected from server!";
    });

    connect(_socket, &QTcpSocket::readyRead, [&]() {
        _buffer.append(_socket->readAll());
        // 循环
        for(;;)
        {
            // 解析包头
            if (!_b_recv_pending)
            {
                // 包头都没发完
                if (_buffer.size() < static_cast<qsizetype>(sizeof(quint32)))
                {
                    return; // 继续等待数据
                }
                // 取包头
                QDataStream stream(&_buffer, QDataStream::ReadOnly);
                stream >> _message_len;
                qDebug() << "数据长度" << _message_len << Qt::endl;
                // 截取数据
                _buffer = _buffer.mid(sizeof(quint32));
            }
            // 不足一条消息则继续等待数据
            if (_buffer.size() < _message_len)
            {
                _b_recv_pending = true;
                return;
            }
            // 截取消息体
            QByteArray msgBody = _buffer.mid(0, _message_len);
            //qDebug() << msgBody << Qt::endl;
            handleMsg(msgBody);
            // 继续读下一条数据
            _buffer = _buffer.mid(_message_len);
            _b_recv_pending = false;
        }
    });

    connect(_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
        qDebug() << "Error:" << _socket->errorString();
    });

    connect(this, &TcpClient::sig_login, this, &TcpClient::slo_login);
    connect(this, &TcpClient::sig_regist, this, &TcpClient::slo_regist);
    connect(this, &TcpClient::sig_findUser, this, &TcpClient::slo_findUser);
    connect(this, &TcpClient::sig_applyFriend, this, &TcpClient::slo_applyFriend);
    connect(this, &TcpClient::sig_recvApplyFriendRequest, this, &TcpClient::slo_recvApplyFriendRequest);
    connect(this, &TcpClient::sig_recvFriends, this, &TcpClient::slo_recvFriends);
    connect(this, &TcpClient::sig_passFriendApply, this, &TcpClient::slo_passFriendApply);
    connect(this, &TcpClient::sig_refuseFriendApply, this, &TcpClient::slo_refuseFriendApply);
    connect(this, &TcpClient::sig_changeProfile, this, &TcpClient::slo_changeProfile);
    connect(this, &TcpClient::sig_removeFriendApply, this, &TcpClient::slo_removeFriendApply);
    connect(this, &TcpClient::sig_removeFriend, this, &TcpClient::slo_removeFriend);
    connect(this, &TcpClient::sig_sendMsg, this, &TcpClient::slo_sendMsg);
    connect(this, &TcpClient::sig_loadWaitMsg, this, &TcpClient::slo_loadWaitMsg);

    // 等待解锁
    init_mutex.lock();
    init_condition.wakeOne();
    init_mutex.unlock();
}

TcpClient& TcpClient::GetInstance()
{
    static TcpClient _instance;
    return _instance;
}

void TcpClient::Login(const QString &user, const QString &pwd)
{
    emit sig_login(user, pwd);
}

void TcpClient::Regist(const QString &user, const QString &pwd, const QString &profileData)
{
    emit sig_regist(user, pwd, profileData);
}

void TcpClient::FindUser(const QString &user)
{
    emit sig_findUser(user);
}

void TcpClient::ApplyFriend(const QString &user, const QString &leave)
{
    emit sig_applyFriend(user, leave);
}

void TcpClient::RecvApplyFriendRequest(const QString &user)
{
    emit sig_recvApplyFriendRequest(user);
}

void TcpClient::RecvFriends(const QString &user)
{
    emit sig_recvFriends(user);
}

void TcpClient::PassFriendApply(const QString &user)
{
    emit sig_passFriendApply(user);
}

void TcpClient::RefuseFriendApply(const QString &user)
{
    emit sig_refuseFriendApply(user);
}

void TcpClient::ChangeProfile(const QString &profile)
{
    emit sig_changeProfile(profile);
}

void TcpClient::RemoveFriendApply(const QString &user)
{
    emit sig_removeFriendApply(user);
}

void TcpClient::RemoveFriend(const QString &user)
{
    emit sig_removeFriend(user);
}

void TcpClient::SendMsg(const QString & msgId, const QString &user, const QString &time, const QString &msg, bool file_msg)
{
    emit sig_sendMsg(msgId, user, time, msg, file_msg);
}

void TcpClient::LoadWaitMsg()
{
    emit sig_loadWaitMsg();
}

void TcpClient::slo_tcp_connect()
{
    qDebug() << "Connecting to server..." << Qt::endl;
    _socket->connectToHost(QHostAddress(StorageManager::GetInstance().serverIp()), StorageManager::GetInstance().serverPort().toInt());
}

void TcpClient::slo_tcp_disconnect()
{
    _socket->disconnectFromHost();
}

void TcpClient::slo_login(const QString &user, const QString &pwd)
{
    qDebug() << "loging..." << Qt::endl;
    QJsonObject json;
    json["type"] = "Login";
    json["user"] = user;
    json["password"] = pwd;
    sendJson(json);
}

void TcpClient::slo_regist(const QString &user, const QString &pwd, const QString &profileData)
{
    qDebug() << "regist..." << Qt::endl;
    QJsonObject json;
    json["type"] = "Regist";
    json["user"] = user;
    json["password"] = pwd;
    json["profile"] = profileData;
    sendJson(json);
}

void TcpClient::slo_findUser(const QString &user)
{
    qDebug() << "find user: " << user << "..." << Qt::endl;
    QJsonObject json;
    json["type"] = "FindUser";
    json["user"] = user;
    sendJson(json);
}

void TcpClient::slo_applyFriend(const QString &user, const QString &leave)
{
    qDebug() << "apply friend: " << user << "..." << Qt::endl;
    QJsonObject json;
    json["type"] = "ApplyFriend";
    json["user"] = user;
    json["leave"] = leave;
    sendJson(json);
}

void TcpClient::slo_recvApplyFriendRequest(const QString &user)
{
    qDebug() << "send receive apply friend request..." << Qt::endl;
    QJsonObject json;
    json["type"] = "RecvApplyFriendRequest";
    json["user"] = user;
    sendJson(json);
}

void TcpClient::slo_recvFriends(const QString &user)
{
    qDebug() << "send receive friends..." << Qt::endl;
    QJsonObject json;
    json["type"] = "RecvFriends";
    json["user"] = user;
    sendJson(json);
}

void TcpClient::slo_passFriendApply(const QString &user)
{
    qDebug() << "pass friend apply..." << Qt::endl;
    QJsonObject json;
    json["type"] = "PassFriendApply";
    json["user"] = user;
    sendJson(json);
}

void TcpClient::slo_refuseFriendApply(const QString &user)
{
    qDebug() << "refuse friend apply..." << Qt::endl;
    QJsonObject json;
    json["type"] = "RefuseFriendApply";
    json["user"] = user;
    sendJson(json);
}

void TcpClient::slo_changeProfile(const QString &profile)
{
    qDebug() << "change user profile..." << Qt::endl;
    QJsonObject json;
    json["type"] = "ChangeProfile";
    json["profile"] = profile;
    sendJson(json);
}

void TcpClient::slo_removeFriendApply(const QString &user)
{
    qDebug() << "remove friend apply" << Qt::endl;
    QJsonObject json;
    json["type"] = "RemoveFriendApply";
    json["user"] = user;
    sendJson(json);
}

void TcpClient::slo_removeFriend(const QString &user)
{
    qDebug() << "remove friend" << Qt::endl;
    QJsonObject json;
    json["type"] = "RemoveFriend";
    json["user"] = user;
    sendJson(json);
}

void TcpClient::slo_sendMsg(const QString & msgId, const QString &user, const QString &time, const QString &msg, bool file_msg)
{
    qDebug() << "tcp thread: " << QThread::currentThread() << Qt::endl;
    qDebug() << "send message" << Qt::endl;
    QJsonObject json;
    json["type"] = "SendMsg";
    json["msgId"] = msgId;
    json["user"] = user;
    json["time"] = time;
    json["msg"] = msg;
    json["file_msg"] = file_msg;
    qDebug() << "json end" << Qt::endl;
    sendJson(json);
    qDebug() << "send end" << Qt::endl;
}

void TcpClient::slo_loadWaitMsg()
{
    qDebug() << "load wait messages..." << Qt::endl;
    QJsonObject json;
    json["type"] = "LoadWaitMsg";
    sendJson(json);
}

void TcpClient::tcp_connect()
{
    emit sig_tcp_connect();
}

void TcpClient::tcp_disconnect()
{
    emit sig_tcp_disconnect();
}
