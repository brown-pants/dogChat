/*  ------------------------------------
 *      工具类
 *  ------------------------------------
 */

#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

class Util
{
public:
    // pinyin
    static char GetInitial(const QString &str);
    static QString Pinyin(const QString &str);

    // file
    static QString GetParentDirectory(const QString &path);
    static QString FileToBase64(const QString &path);
    static QByteArray FileToByteArray(const QString &path);
    static bool WriteToFile(const QString &path, const QByteArray &data);

    // json
    static void JsonDocToObj(const QJsonDocument &jsonDoc, std::function<void(QJsonObject)> success, std::function<void()> fail = [](){ qDebug() << "error in JsonDocToObj" << Qt::endl; });
    static void JsonDocToArray(const QJsonDocument &jsonDoc, std::function<void(QJsonArray)> success, std::function<void()> fail = [](){ qDebug() << "error in JsonDocToArray" << Qt::endl; });
    static void JsonValueToObj(const QJsonValue &jsonValue, std::function<void(QJsonObject)> success, std::function<void()> fail = [](){ qDebug() << "error in JsonValueToObj" << Qt::endl; });
    static void JsonValueToString(const QJsonValue &jsonValue, std::function<void(QString)> success, std::function<void()> fail = [](){ qDebug() << "error in JsonValueToString" << Qt::endl; });
    static void JsonValueToArray(const QJsonValue &jsonValue, std::function<void(QJsonArray)> success, std::function<void()> fail = [](){ qDebug() << "error in JsonValueToArray" << Qt::endl; });

private:
    Util();
};

#endif // UTIL_H
