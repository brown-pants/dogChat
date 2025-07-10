#ifndef __MYSQLDAO_H__
#define __MYSQLDAO_H__
#include "../pool/MySQLPool.h"

class MysqlDao
{
public:
    static MysqlDao &GetInstance();
    ~MysqlDao();
    int RegUser(const std::string& name, const std::string& pwd, const std::string &profile_url);
    int Login(const std::string& name, const std::string& pwd);
    std::string GetProfileUrl(const std::string& name);

private:
    std::unique_ptr<MySQLPool> mysql_pool;
    MysqlDao();
};

#endif