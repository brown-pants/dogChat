#include "MySQLDao.h"
#include "../config/Config.h"

MysqlDao &MysqlDao::GetInstance()
{
    static MysqlDao dao;
    return dao;
}

MysqlDao::MysqlDao()
{
    const std::string& address = Config::mysql_address();
    const std::string& port = Config::mysql_port();
    const std::string& pwd = Config::mysql_password();
    const std::string& user = Config::mysql_user();
    const std::string& schema = Config::mysql_schema();
    mysql_pool.reset(new MySQLPool("tcp://" + address + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao()
{
    mysql_pool->close();
}

int MysqlDao::RegUser(const std::string& name, const std::string& pwd, const std::string &profile_url)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }

    // 执行过程
    if (!con->executePrepared("CALL reg_user(?, ?, ?, @result)", {name, pwd, profile_url}))
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }
    
    try {
        // 读取 result
        std::unique_ptr<sql::ResultSet> res = con->executePreparedQuery("SELECT @result AS result");
        if (res->next()) 
        {
            int result = res->getInt("result");
            mysql_pool->returnConnection(std::move(con));
            return result;
        }
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }
    return 1;
}

int MysqlDao::Login(const std::string& name, const std::string& pwd)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }

    try {
        std::unique_ptr<sql::ResultSet> res = con->executePreparedQuery("select password from users where name = ?", {name});
        mysql_pool->returnConnection(std::move(con));
        if (res->next()) 
        {
            const std::string &password = res->getString("password");
            if (password == pwd) return 1;
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }
    return 0;
}

std::string MysqlDao::GetProfileUrl(const std::string& name)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return "";
    }
    try {
        std::unique_ptr<sql::ResultSet> res = con->executePreparedQuery("select profile_url from users where name = ?", {name});
        mysql_pool->returnConnection(std::move(con));
        if (res->next()) 
        {
            const std::string &url = res->getString("profile_url");
            return url;
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        mysql_pool->returnConnection(std::move(con));
    }
    return "";
}