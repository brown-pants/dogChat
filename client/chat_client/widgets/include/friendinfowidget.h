#ifndef FRIENDINFOWIDGET_H
#define FRIENDINFOWIDGET_H

#include <QWidget>

namespace Ui {
class FriendInfoWidget;
}

class FriendInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FriendInfoWidget(QWidget *parent = nullptr);
    QString curUser() const;
    void setUser(const QString &user, const QPixmap &profile);
    ~FriendInfoWidget();

signals:
    void toChat(const QString &user);
    void removeFriend();

private slots:
    void on_settingButton_clicked();

    void on_chatButton_clicked();

private:
    Ui::FriendInfoWidget *ui;
    QString user;
};

#endif // FRIENDINFOWIDGET_H
