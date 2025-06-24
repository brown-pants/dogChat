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

protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::ChatMsgItem *ui;

    static QIcon exclamationIcon;

    bool hover;
    bool myMsg;
    int lastDocHeight;
    Type msgType;
    QColor bubbleColor;
    QListWidgetItem *listWidgetItem;
};

#endif // CHATMSGITEM_H
