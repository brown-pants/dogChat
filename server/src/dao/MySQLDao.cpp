#include "MySQLDao.h"
#include "../config/Config.h"
#include "../util/Util.h"

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
    }
    return -1;
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
            if (res->getString("password") == pwd) return 1;
        }
        return 0;
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        mysql_pool->returnConnection(std::move(con));
    }
    return -1;
}

int MysqlDao::FindUser(const std::string& name)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }

    try {
        std::unique_ptr<sql::ResultSet> res = con->executePreparedQuery("select * from users where name = ?", {name});
        mysql_pool->returnConnection(std::move(con));
        if (res->next()) 
        {
            return 1;
        }
        return 0;
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        mysql_pool->returnConnection(std::move(con));
    }
    return -1;
}

int MysqlDao::ApplyFriend(const std::string& user_apply, const std::string& user_accept, const std::string& leave_msg)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }
    
    // 执行过程
    if (!con->executePrepared("CALL apply_friend(?, ?, ?, @result)", {user_apply, user_accept, leave_msg}))
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
    }
    return -1;
}

int MysqlDao::ChangeFriendApplyState(const std::string& user_apply, const std::string& user_accept, const std::string& state)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }
    
    // 执行过程
    if (!con->executePrepared("CALL change_friend_apply_state(?, ?, ?, @result)", {user_apply, user_accept, state}))
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
    }
    return -1;
}

nlohmann::json MysqlDao::GetFriendApplyList(const std::string& name)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return nlohmann::json::array();
    }
    try {
        nlohmann::json json_arr = nlohmann::json::array();
        std::unique_ptr<sql::ResultSet> res = con->executePreparedQuery("SELECT user_apply, leave_msg, apply_state FROM friend_apply WHERE user_accept = ?", {name});
        while (res->next()) 
        {
            nlohmann::json item;
            std::string user_apply = res->getString("user_apply");
            item["user_apply"] = user_apply;
            item["leave_msg"] = res->getString("leave_msg");
            item["apply_state"] = res->getString("apply_state");
            item["profile"] = Util::FileToBase64(GetProfileUrl(user_apply));
            json_arr.push_back(item);
        }
        mysql_pool->returnConnection(std::move(con));
        return json_arr;
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        mysql_pool->returnConnection(std::move(con));
    }
    return nlohmann::json::array();
}

nlohmann::json MysqlDao::GetFriends(const std::string& name)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return nlohmann::json::array();
    }
    try {
        nlohmann::json json_arr = nlohmann::json::array();
        std::unique_ptr<sql::ResultSet> res = con->executePreparedQuery("SELECT userB FROM friends WHERE userA = ?", {name});
        while (res->next()) 
        {
            nlohmann::json item;
            std::string userB = res->getString("userB");
            item["user"] = userB;
            item["profile"] = Util::FileToBase64(GetProfileUrl(userB));
            json_arr.push_back(item);
        }
        mysql_pool->returnConnection(std::move(con));
        return json_arr;
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        mysql_pool->returnConnection(std::move(con));
    }
    return nlohmann::json::array();
}

int MysqlDao::IsFriend(const std::string &userA, const std::string &userB)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }

    try {
        std::unique_ptr<sql::ResultSet> res = con->executePreparedQuery("SELECT * FROM friends WHERE userA = ? AND userB = ?", {userA, userB});
        mysql_pool->returnConnection(std::move(con));
        if (res->next()) 
        {
            return 1;
        }
        return 0;
    }
    catch (sql::SQLException& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        mysql_pool->returnConnection(std::move(con));
    }
    return -1;
}

int MysqlDao::SetFriend(const std::string &userA, const std::string &userB)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }
    
    // 执行过程
    if (!con->executePrepared("CALL add_friend(?, ?, @result)", {userA, userB}))
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
    }
    return -1;
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
            return res->getString("profile_url");
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

int MysqlDao::RemoveFriendApply(const std::string apply_user, const std::string accept_user)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }
    
    // 执行过程
    if (!con->executePrepared("CALL remove_friend_apply(?, ?, @result)", {apply_user, accept_user}))
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
    }
    return -1;
}

int MysqlDao::RemoveFriend(const std::string &user1, const std::string &user2)
{
    MySQLPool::MySQLConnPtr con = mysql_pool->getConnection();
    if (con == nullptr) 
    {
        mysql_pool->returnConnection(std::move(con));
        return -1;
    }
    
    // 执行过程
    if (!con->executePrepared("CALL remove_friend(?, ?, @result)", {user1, user2}))
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
    }
    return -1;
}