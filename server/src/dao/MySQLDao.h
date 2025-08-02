#ifndef __MYSQLDAO_H__
#define __MYSQLDAO_H__
#include "../pool/MySQLPool.h"
#include <nlohmann/json.hpp>

class MysqlDao
{
public:
    static MysqlDao &GetInstance();
    ~MysqlDao();
    int RegUser(const std::string& name, const std::string& pwd, const std::string &profile_url);
    int Login(const std::string& name, const std::string& pwd);
    int FindUser(const std::string& name);
    int ApplyFriend(const std::string& user_apply, const std::string& user_accept, const std::string& leave_msg);
    int ChangeFriendApplyState(const std::string& user_apply, const std::string& user_accept, const std::string& state);
    std::string GetProfileUrl(const std::string& name);
    nlohmann::json GetFriendApplyList(const std::string& name);
    nlohmann::json GetFriends(const std::string& name);
    int IsFriend(const std::string &userA, const std::string &userB);
    int SetFriend(const std::string &userA, const std::string &userB);
    int RemoveFriendApply(const std::string apply_user, const std::string accept_user);
    int RemoveFriend(const std::string &user1, const std::string &user2);

private:
    std::unique_ptr<MySQLPool> mysql_pool;
    MysqlDao();
};

#endif