/*  ------------------
 *      主窗体类
 *  ------------------
 */

#ifndef MAINWND_H
#define MAINWND_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWnd;
}
QT_END_NAMESPACE

class MainWnd : public QWidget
{
    Q_OBJECT

public:
    static MainWnd &GetInstance(const QString &user = "", const QPixmap &profile = QPixmap());
    ~MainWnd();

private slots:
    void on_chatButton_clicked();

    void on_friendButtom_clicked();

private:
    Ui::MainWnd *ui;
    QString curr_user;
    QPixmap user_profile;
    MainWnd(const QString &user, const QPixmap &profile, QWidget *parent = nullptr);
};
#endif // MAINWND_H
