/*  ------------------------------------
 *      工具类
 *  ------------------------------------
 */

#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <functional>

class Util
{
public:
    static char GetInitial(const QString &str);
    static QString Pinyin(const QString &str);
    static QString GetParentDirectory(const QString &path);

    // json
    static void JsonDocToObj(const QJsonDocument &jsonDoc, std::function<void(QJsonObject)> success, std::function<void()> fail = [](){});
    static void JsonValueToObj(const QJsonValue &jsonValue, std::function<void(QJsonObject)> success, std::function<void()> fail = [](){});
    static void JsonValueToString(const QJsonValue &jsonValue, std::function<void(QString)> success, std::function<void()> fail = [](){});

private:
    Util();
};

#endif // UTIL_H
