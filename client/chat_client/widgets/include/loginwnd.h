/*  ------------------
 *      登陆界面类
 *  ------------------
 */

#ifndef LOGINWND_H
#define LOGINWND_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

namespace Ui {
class LoginWnd;
}

class LoginWnd : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(int rotation READ getRotation WRITE setRotation)

public:
    explicit LoginWnd(QWidget *parent = nullptr);
    ~LoginWnd();

    int getRotation() const;
    void setRotation(int newRotation);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::LoginWnd *ui;
    QMovie *topMovie;
    QPushButton *profileButton;
    QLabel *loggingLabel;
    QTimer *loggingTimer;
    QPushButton *rtnBtn;
    QPropertyAnimation *rotateAnim;
    QVariantAnimation *slideAnim;
    int rotation;
    QPixmap LockPixmap;
    QPixmap LockOpenPixmap;
    QPixmap registerProfile;
    QString profilePath;
    bool servConnSucc;

private slots:
    void onFocusChanged(QWidget *old, QWidget *now);
    void on_toRegisterButton_clicked();
    void on_toLoginButton_clicked();
    void on_toSettingButton_clicked();
    void chooseProfile();
    void login();
    void regist();
};

#endif // LOGINWND_H
