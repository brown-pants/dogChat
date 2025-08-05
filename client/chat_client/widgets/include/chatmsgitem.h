/*  ------------------------------------
 *      消息框类
 *  ------------------------------------
 */

#ifndef CHATMSGITEM_H
#define CHATMSGITEM_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class ChatMsgItem;
}

class ChatMsgItem : public QWidget
{
    Q_OBJECT

public:
    enum Type
    {
        TextMsg,    // 文本消息
        ImageMsg,   // 图片消息
        FileMsg     // 文件消息
    };

    explicit ChatMsgItem(bool myMsg, const QPixmap &profile, QListWidgetItem *listWidgetItem, QWidget *parent = nullptr);
    ~ChatMsgItem();

    void setText(const QString &text);
    void setFile(const QString &file_url);

    void showExclamationButton();

signals:
    void resend();

protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_exclamationButton_clicked();

private:
    Ui::ChatMsgItem *ui;

    static QIcon exclamationIcon;

    bool hover;     // 鼠标是否在消息框上方
    bool myMsg;     // 是否是自己发送的消息
    Type msgType;   // 消息类型
    QColor bubbleColor;
    QListWidgetItem *listWidgetItem;

    void openFile();
    void openDir();
};

#endif // CHATMSGITEM_H
