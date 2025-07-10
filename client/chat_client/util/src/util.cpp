#include "util.h"
#include "pinyin4cpp/PinyinHelper.h"
#include <QDebug>
#include <QDir>
#include <QBuffer>
#include <QPixmap>

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

QString Util::GetParentDirectory(const QString &path)
{
    QDir dir(path);
    // 跳转到上级目录
    if (dir.cdUp())
    {
        return dir.absolutePath(); // 返回绝对路径
    }
    else
    {
        qDebug() << "无法获取上级目录:" << path;
    }
    return "";
}

QString Util::FileToBase64(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "无法打开文件:" << path;
        return QString();
    }

    QByteArray imageData = file.readAll();
    return QString::fromLatin1(imageData.toBase64().data());
}

bool Util::WriteToFile(const QString &path, const QByteArray &data)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << "无法打开文件:" << path;
        return false;
    }
    return file.write(data);
}

void Util::JsonDocToObj(const QJsonDocument &jsonDoc, std::function<void (QJsonObject)> success, std::function<void ()> fail)
{
    if (jsonDoc.isObject())
    {
        success(jsonDoc.object());
    }
    else fail();
}

void Util::JsonValueToObj(const QJsonValue &jsonValue, std::function<void (QJsonObject)> success, std::function<void ()> fail)
{
    if (jsonValue.isObject())
    {
        success(jsonValue.toObject());
    }
    else fail();
}

void Util::JsonValueToString(const QJsonValue &jsonValue, std::function<void (QString)> success, std::function<void ()> fail)
{
    if (jsonValue.isString())
    {
        success(jsonValue.toString());
    }
    else fail();
}

Util::Util() {}
