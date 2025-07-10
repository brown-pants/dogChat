#ifndef __MYSQLPOOL_H__
#define __MYSQLPOOL_H__
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

class MySQLConnection
{
public:
    MySQLConnection(const std::string &url, const std::string &user, const std::string &password, const std::string &schema);
    ~MySQLConnection();

    bool connect(const std::string &url, const std::string &user, const std::string &password, const std::string &schema);
    bool disconnect();
    
    bool executePrepared(const std::string& sql, const std::vector<std::string>& params = std::vector<std::string>());
    std::unique_ptr<sql::ResultSet> executePreparedQuery(const std::string& sql, const std::vector<std::string>& params = std::vector<std::string>());

private:
    std::unique_ptr<sql::Connection> con;
};

class MySQLPool
{
public:
    using MySQLConnPtr = std::unique_ptr<MySQLConnection>;
    MySQLPool(const std::string &url, const std::string &user, const std::string &password, const std::string &schema, int poolSize);
    ~MySQLPool();
    MySQLConnPtr getConnection();
    void returnConnection(MySQLConnPtr con);
    void close();

private:
    std::string url;
    std::string user;
    std::string password;
    std::string schema;
    int poolSize;
    std::queue<MySQLConnPtr> pool;
    std::mutex mutex;
    std::condition_variable condition;
    bool stopped;
};

#endif