#ifndef UTIL_H
#define UTIL_H

#include <QString>

class Util
{
public:
    static char GetInitial(const QString &str);
    static QString Pinyin(const QString &str);

private:
    Util();
};

#endif // UTIL_H
