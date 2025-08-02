#ifndef APPLYFRIENDDIALOG_H
#define APPLYFRIENDDIALOG_H

#include <QDialog>

namespace Ui {
class ApplyFriendDialog;
}

class ApplyFriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriendDialog(QWidget *parent = nullptr);
    ~ApplyFriendDialog();

    void setUser(const QString &user);

private slots:
    void on_cancelButton_clicked();

    void on_applyButton_clicked();

private:
    Ui::ApplyFriendDialog *ui;
    QString target_user;
};

#endif // APPLYFRIENDDIALOG_H
