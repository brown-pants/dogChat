/*  ------------------------------------
 *      表情选择界面类
 *  ------------------------------------
 */

#ifndef CHOOSEEMOJIWIDGET_H
#define CHOOSEEMOJIWIDGET_H

#include <QWidget>

namespace Ui {
class ChooseEmojiWidget;
}

class ChooseEmojiWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChooseEmojiWidget(QWidget *parent = nullptr);
    ~ChooseEmojiWidget();

signals:
    void charEmojiClicked(const QString &emoji);
    void animEmojiClicked(const QString &url);

protected:
    void paintEvent(QPaintEvent *e);

private slots:
    void on_charEmojiButton_clicked();

    void on_animEmojiButton_clicked();

private:
    Ui::ChooseEmojiWidget *ui;

    void loadCharEmojis();
    void loadAnimEmojis();
};

#endif // CHOOSEEMOJIWIDGET_H
