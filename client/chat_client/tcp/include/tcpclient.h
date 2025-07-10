#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    static TcpClient& GetInstance();
    void Login(const QString &user, const QString &pwd);
    void Regist(const QString &user, const QString &pwd, const QString &profileData);

public slots:
    void tcp_connect();
    void tcp_disconnect();
    void slo_login(const QString &user, const QString &pwd);
    void slo_regist(const QString &user, const QString &pwd, const QString &profileData);


signals:
    void connected();
    void sig_login(const QString &user, const QString &pwd);
    void sig_regist(const QString &user, const QString &pwd, const QString &profileData);
    void sig_login_over(const QString &status, const QString &profile);
    void sig_regist_over(const QString &status);


private:
    explicit TcpClient(QObject *parent = nullptr);
    QByteArray toTcpData(QByteArray data);
    void handleMsg(const QByteArray &body);
    QTcpSocket _socket;

    QByteArray _buffer;     // 真实数据
    // 前 4Byte 为包头
    quint32 _message_len;   // 数据长度
    bool _b_recv_pending;

signals:
};

#endif // TCPCLIENT_H
