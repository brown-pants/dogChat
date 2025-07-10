#include "tcpclient.h"
#include "storagemanager.h"
#include "util.h"
#include <QJsonDocument>
#include <QJsonObject>

TcpClient::TcpClient(QObject *parent)
    : QObject{parent}, _b_recv_pending(false)
{
    connect(&_socket, &QTcpSocket::connected, [&]() {
        qDebug() << "Connected to server!";
        emit connected();
    });

    connect(&_socket, &QTcpSocket::disconnected, [&]() {
        qDebug() << "Disconnected from server!";
    });

    connect(&_socket, &QTcpSocket::readyRead, [&]() {
        _buffer.append(_socket.readAll());
        QDataStream stream(&_buffer, QDataStream::ReadOnly);
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
                stream >> _message_len;
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

    connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
          Q_UNUSED(socketError)
          qDebug() << "Error:" << _socket.errorString();
    });

    connect(this, &TcpClient::sig_login, this, &TcpClient::slo_login);
    connect(this, &TcpClient::sig_regist, this, &TcpClient::slo_regist);
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
        });
    });
}

TcpClient& TcpClient::GetInstance()
{
    static TcpClient instance;
    return instance;
}

void TcpClient::Login(const QString &user, const QString &pwd)
{
    emit sig_login(user, pwd);
}

void TcpClient::Regist(const QString &user, const QString &pwd, const QString &profileData)
{
    emit sig_regist(user, pwd, profileData);
}

void TcpClient::slo_login(const QString &user, const QString &pwd)
{
    qDebug() << "loging..." << Qt::endl;
    QJsonObject json;
    json["type"] = "Login";
    json["user"] = user;
    json["password"] = pwd;
    QByteArray jsonByte = QJsonDocument(json).toJson(QJsonDocument::Compact);
    QByteArray data = toTcpData(jsonByte);
    _socket.write(data);
}

void TcpClient::slo_regist(const QString &user, const QString &pwd, const QString &profileData)
{
    qDebug() << "regist..." << Qt::endl;
    QJsonObject json;
    json["type"] = "Regist";
    json["user"] = user;
    json["password"] = pwd;
    json["profile"] = profileData;
    QByteArray jsonByte = QJsonDocument(json).toJson(QJsonDocument::Compact);
    QByteArray data = toTcpData(jsonByte);
    _socket.write(data);
}

void TcpClient::tcp_connect()
{
    qDebug() << "Connecting to server..." << Qt::endl;
    _socket.connectToHost(QHostAddress(StorageManager::GetInstance().serverIp()), StorageManager::GetInstance().serverPort().toInt());
}

void TcpClient::tcp_disconnect()
{
    _socket.disconnectFromHost();
}
