#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QThread>
#include "global.h"

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient();
    ~TcpClient();
    static TcpClient& GetInstance();
    void Login(const QString &user, const QString &pwd);
    void Regist(const QString &user, const QString &pwd, const QString &profileData);
    void FindUser(const QString &user);
    void ApplyFriend(const QString &user, const QString &leave);
    void RecvApplyFriendRequest(const QString &user);
    void RecvFriends(const QString &user);
    void PassFriendApply(const QString &user);
    void RefuseFriendApply(const QString &user);
    void ChangeProfile(const QString &profile);
    void RemoveFriendApply(const QString &user);
    void RemoveFriend(const QString &user);
    void SendMsg(const QString & msgId, const QString &user, const QString &time, const QString &msg, bool file_msg);
    void LoadWaitMsg();

    void tcp_connect();
    void tcp_disconnect();

public slots:
    void init();
    void slo_tcp_connect();
    void slo_tcp_disconnect();
    void slo_login(const QString &user, const QString &pwd);
    void slo_regist(const QString &user, const QString &pwd, const QString &profileData);
    void slo_findUser(const QString &user);
    void slo_applyFriend(const QString &user, const QString &leave);
    void slo_recvApplyFriendRequest(const QString &user);
    void slo_recvFriends(const QString &user);
    void slo_passFriendApply(const QString &user);
    void slo_refuseFriendApply(const QString &user);
    void slo_changeProfile(const QString &profile);
    void slo_removeFriendApply(const QString &user);
    void slo_removeFriend(const QString &user);
    void slo_sendMsg(const QString & msgId, const QString &user, const QString &time, const QString &msg, bool file_msg);
    void slo_loadWaitMsg();

signals:
    void connected();
    void sig_tcp_connect();
    void sig_tcp_disconnect();
    void sig_login(const QString &user, const QString &pwd);
    void sig_login_over(const QString &status, const QString &profile);
    void sig_regist(const QString &user, const QString &pwd, const QString &profileData);
    void sig_regist_over(const QString &status);
    void sig_findUser(const QString &user);
    void sig_findUser_over(const QString &status, const QString &profile, const QString &isFriend);
    void sig_applyFriend(const QString &user, const QString &leave);
    void sig_applyFriend_over(const QString &status);
    void sig_applyFriendRequest(const QString &user, const QString &leave, const QString &profile);
    void sig_recvApplyFriendRequest(const QString &user);
    void sig_recvApplyFriendRequest_over(const QVector<ApplyFriendInfo> &apply_arr);
    void sig_recvFriends(const QString &user);
    void sig_recvFriends_over(const QVector<FriendInfo> &friend_arr);
    void sig_passFriendApply(const QString &user);
    void sig_refuseFriendApply(const QString &user);
    void sig_addedFriend_over(const QString &user, const QString &profile);
    void sig_changeProfile(const QString &profile);
    void sig_removeFriendApply(const QString &user);
    void sig_removeFriend(const QString &user);
    void sig_removeFriend_over(const QString &user);
    void sig_sendMsg(const QString & msgId, const QString &user, const QString &time, const QString &msg, bool file_msg);
    void sig_recvMsg(const QString &user, const QString &time, const QString &msg, bool file_msg);
    void sig_loadWaitMsg();
    void sig_offline();

private:
    QThread *_thread;

    QByteArray toTcpData(QByteArray data);
    void sendJson(const QJsonObject &json);
    void handleMsg(const QByteArray &body);
    QTcpSocket *_socket;

    QByteArray _buffer;     // 真实数据
    // 前 4Byte 为包头
    quint32 _message_len;   // 数据长度
    bool _b_recv_pending;

signals:
};

#endif // TCPCLIENT_H
