/*  ------------------
 *      程序初始化类
 *  ------------------
 *  功能：
 *      1.设置全局样式表
 *      2.设置全局事件过滤器
 */

#ifndef APPINIT_H
#define APPINIT_H

#include <QObject>

class AppInit : public QObject
{
    Q_OBJECT
public:
    static void Init();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    //窗口伸缩方向
    enum ResizeArea {
        ResizeNone = 0x0,   //无
        ResizeLeft = 0x1,   //左
        ResizeRight = 0x2,  //右
        ResizeTop = 0x4,    //上
        ResizeBottom = 0x8  //下
    };

    explicit AppInit(QObject *parent = nullptr);

signals:
};

#endif // APPINIT_H
