/*  ------------------------------------
 *      控制栏类(最小化 | 最大化 | 关闭)
 *  ------------------------------------
 */

#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <QWidget>
#include <QIcon>

namespace Ui {
class ControlWidget;
}

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControlWidget(QWidget *parent = nullptr);
    ~ControlWidget();

    void setControlWnd(QWidget *w);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void on_closeButton_clicked();

    void on_maximizeButton_clicked();

    void on_minimizeButton_clicked();

private:
    Ui::ControlWidget *ui;
    QRect wndNormalGeometry;
    QWidget *controlWnd;
    bool isNormalState;
    QIcon normalIcon, maximizeIcon;
};

#endif // CONTROLWIDGET_H
