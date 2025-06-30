#ifndef LOGINWND_H
#define LOGINWND_H

#include <QWidget>
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
    QPropertyAnimation *rotateAnim;
    int rotation;
    QPixmap LockPixmap;
    QPixmap LockOpenPixmap;


private slots:
    void onFocusChanged(QWidget *old, QWidget *now);
    void on_toRegisterButton_clicked();
    void on_toLoginButton_clicked();
    void on_toSettingButton_clicked();
};

#endif // LOGINWND_H
