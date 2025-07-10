#include "MySQLPool.h"

MySQLConnection::MySQLConnection(const std::string &url, const std::string &user, const std::string &password, const std::string &schema)
{
    connect(url, user, password, schema);
}

MySQLConnection::~MySQLConnection()
{
    disconnect();
}

bool MySQLConnection::connect(const std::string &url, const std::string &user, const std::string &password, const std::string &schema)
{
    try {
        sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
        con = std::unique_ptr<sql::Connection>(driver->connect(url, user, password));
        con->setSchema(schema);
    }
    catch (sql::SQLException &e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        return false;
    }
    return true;
}

bool MySQLConnection::disconnect()
{
    try {
        if (!con->isClosed()) con->close();
    }
    catch (sql::SQLException &e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        return false;
    }
    return true;
}

std::unique_ptr<sql::ResultSet> MySQLConnection::executePreparedQuery(const std::string& sql, const std::vector<std::string>& params)
{
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(sql));
        for (size_t i = 0; i < params.size(); i ++) 
        {
            pstmt->setString(i + 1, params[i]);
        }
        return std::unique_ptr<sql::ResultSet>(pstmt->executeQuery());
    } 
    catch (const sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return nullptr;
}

bool MySQLConnection::executePrepared(const std::string& sql, const std::vector<std::string>& params)
{
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(sql));
        for (size_t i = 0; i < params.size(); i ++) 
        {
            pstmt->setString(i + 1, params[i]);
        }
        pstmt->execute();
        return true;
    } 
    catch (const sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return false;
}

MySQLPool::MySQLPool(const std::string &url, const std::string &user, const std::string &password, const std::string &schema, int poolSize)
    : url(url), user(user), password(password), schema(schema), poolSize(poolSize), stopped(false)
{
    for (int i = 0; i < poolSize; i ++) 
    {
        MySQLConnPtr con = std::make_unique<MySQLConnection>(url, user, password, schema);
        pool.push(std::move(con));
    }
}

MySQLPool::~MySQLPool()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (!pool.empty()) {
        pool.pop();
    }
}

MySQLPool::MySQLConnPtr MySQLPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex);
    // wait 执行函数 1.如果返回 true 继续往下执行， false则解锁lock 然后等待 notify后执行 1操作
    condition.wait(lock, [this] { 
        if (stopped) 
        {
            return true;
        }        
        return !pool.empty(); 
    });
    if (stopped) 
    {
        return nullptr;
    }
    MySQLConnPtr con(std::move(pool.front()));
    pool.pop();
    return con;
}

void MySQLPool::returnConnection(MySQLPool::MySQLConnPtr con) 
{
    std::unique_lock<std::mutex> lock(mutex);
    if (stopped) 
    {
        return;
    }
    pool.push(std::move(con));
    condition.notify_one();
}

void MySQLPool::close()
{
    stopped = true;
    condition.notify_all();
}