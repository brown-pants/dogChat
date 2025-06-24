#include "util.h"
#include "pinyin4cpp/PinyinHelper.h"

char Util::GetInitial(const QString &str)
{
    QChar wstr = str.at(0).unicode();
    // 中文
    if ((wstr >= (QChar)0x4E00 && wstr <= (QChar)0x9FFF) || (wstr >= (QChar)0x3400 && wstr <= (QChar)0x4DBF))
    {
        QList<QString> pinyinList;
        HanyuPinyinOutputFormat *format = new HanyuPinyinOutputFormat();
        PinyinHelper::toHanyuPinyinStringArray(wstr, format, &pinyinList);
        if (!pinyinList.empty())
        {
            QString py = QString::fromStdString(pinyinList[0].toStdString());
            return py[0].toUpper().toLatin1();
        }
        else
        {
            qDebug() << "中文转拼音失败" << Qt::endl;
        }
    }
    // 英文
    return wstr.toUpper().toLatin1();
}

QString Util::Pinyin(const QString &str)
{
    QString pinyin;
    for (int i = 0; i < str.length(); i ++)
    {
        QChar wstr = str.at(0).unicode();
        // 中文
        if ((wstr >= (QChar)0x4E00 && wstr <= (QChar)0x9FFF) || (wstr >= (QChar)0x3400 && wstr <= (QChar)0x4DBF))
        {
            QList<QString> pinyinList;
            HanyuPinyinOutputFormat *format = new HanyuPinyinOutputFormat();
            PinyinHelper::toHanyuPinyinStringArray(wstr, format, &pinyinList);
            if (!pinyinList.empty())
            {
                QString py = QString::fromStdString(pinyinList[0].toStdString());
                pinyin += py;
            }
            else
            {
                qDebug() << "中文转拼音失败" << Qt::endl;
            }
        }
        // 英文
        pinyin += wstr.toLatin1();
    }
    return pinyin;
}

Util::Util() {}
