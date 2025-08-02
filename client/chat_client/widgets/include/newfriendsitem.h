#ifndef NEWFRIENDSITEM_H
#define NEWFRIENDSITEM_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class NewFriendsItem;
}

class NewFriendsItem : public QWidget
{
    Q_OBJECT

public:
    explicit NewFriendsItem(const QPixmap &profile, const QString &friendName, const QString &leave_msg, const QString &apply_state, QListWidgetItem *item, QListWidget *listWidget);
    ~NewFriendsItem();

protected:
    void resizeEvent(QResizeEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private slots:
    void on_passButton_clicked();

    void on_refuseButton_clicked();

private:
    Ui::NewFriendsItem *ui;
    QString name, leave;
    QListWidgetItem *m_item;
    QListWidget *m_listWidget;
};

#endif // NEWFRIENDSITEM_H
