#ifndef NEWFRIENDSWIDGET_H
#define NEWFRIENDSWIDGET_H

#include <QWidget>

namespace Ui {
class NewFriendsWidget;
}

class NewFriendsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NewFriendsWidget(QWidget *parent = nullptr);
    ~NewFriendsWidget();

signals:
    void sig_insertFriendItem(const QPixmap &profile, const QString &friendName, const QString &leave_msg, const QString &apply_state, int row = -1);

private slots:
    void insertFriendItem(const QPixmap &profile, const QString &friendName, const QString &leave_msg, const QString &apply_state, int row = -1);

private:
    Ui::NewFriendsWidget *ui;

};

#endif // NEWFRIENDSWIDGET_H
