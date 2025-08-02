#ifndef FINDUSERWIDGET_H
#define FINDUSERWIDGET_H

#include <QWidget>
#include "applyfrienddialog.h"

namespace Ui {
class FindUserWidget;
}

class FindUserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FindUserWidget(QWidget *parent = nullptr);
    ~FindUserWidget();
    void setInfo(const QString &name, const QPixmap &profile, bool isFriend);

signals:
    void profileChanged(const QString &profilePath);
    void toChat(const QString user);

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private slots:
    void on_button_clicked();

private:
    Ui::FindUserWidget *ui;
    QString m_user;
    ApplyFriendDialog *applyDialog;
};

#endif // FINDUSERWIDGET_H
