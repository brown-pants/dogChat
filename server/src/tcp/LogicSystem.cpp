#include "LogicSystem.h"
#include "../dao/MySQLDao.h"
#include "../util/Util.h"
#include "Server.h"
#include <iostream>
#include <filesystem>

LogicSystem::LogicSystem()
    : isStop(false), thread(&LogicSystem::dealMsgs, this)
{

}

LogicSystem &LogicSystem::GetInstance()
{
    static LogicSystem instance;
    return instance;
}

void LogicSystem::stop()
{
    cv.notify_one();
    isStop = true;
    thread.join();
}

void LogicSystem::pushMsg(std::shared_ptr<Session> session, const std::string &json_str)
{
    nlohmann::json json = nlohmann::json::parse(json_str);
    try {
        const std::string &type = json["type"];
        if (type == "Login")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::Login, this, json["user"], json["password"], session));
            cv.notify_one();
        }
        else if (type == "Regist")
        {
            std::unique_lock<std::mutex> lock(mtx);
            // base64 To binary
            std::vector<u_char> profile = Util::Base64ToBinary(json["profile"]);
            // push msg
            m_msgs.emplace(std::bind(&LogicSystem::Regist, this, json["user"], json["password"], profile, session));
            cv.notify_one();
        }
        else if (type == "FindUser")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::FindUser, this, json["user"], session));
            cv.notify_one();
        }
        else if (type == "ApplyFriend")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::ApplyFriend, this, json["user"], json["leave"], session));
            cv.notify_one();
        }
        else if (type == "ApplyFriendRequires")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::ApplyFriendRequest, this, json["apply_user"], json["leave"], session));
            cv.notify_one();
        }
        else if (type == "RecvApplyFriendRequest")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::RecvApplyFriendRequest, this, json["user"], session));
            cv.notify_one();
        }
        else if (type == "RecvFriends")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::RecvFriends, this, json["user"], session));
            cv.notify_one();
        }
        else if (type == "PassFriendApply")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::PassFriendApply, this, json["user"], session));
            cv.notify_one();
        }
        else if (type == "RefuseFriendApply")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::RefuseFriendApply, this, json["user"], session));
            cv.notify_one();
        }
        else if (type == "ChangeProfile")
        {
            std::unique_lock<std::mutex> lock(mtx);
            std::vector<u_char> profile  = Util::Base64ToBinary(json["profile"]);
            m_msgs.emplace(std::bind(&LogicSystem::ChangeProfile, this, profile, session));
            cv.notify_one();
        }
        else if (type == "RemoveFriendApply")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::RemoveFriendApply, this, json["user"], session));
            cv.notify_one();
        }
        else if (type == "RemoveFriend")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::RemoveFriend, this, json["user"], session));
            cv.notify_one();
        }
        else if (type == "SendMsg")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::SendMsg, this, json["msgId"], json["user"], json["time"], json["msg"], json["file_msg"], session));
            cv.notify_one();
        }
        else if (type == "LoadWaitMsg")
        {
            std::unique_lock<std::mutex> lock(mtx);
            m_msgs.emplace(std::bind(&LogicSystem::LoadWaitMsg, this, session));
            cv.notify_one();
        }
    }
    catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n" << "Byte position: " << e.byte << std::endl;
    }
}

void LogicSystem::dealMsgs()
{
    for(;;)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this](){
            return !m_msgs.empty() || isStop;
        });
        if (!m_msgs.empty())
        {
            std::function<void()> func(std::move(m_msgs.front()));
            m_msgs.pop();
            lock.unlock();
            func();
        }
        else break;
    }
}

std::string LogicSystem::toTcpData(const std::string body)
{
    uint32_t bodyLen = boost::asio::detail::socket_ops::host_to_network_long(body.length());
    const char *bodyLen_ptr = reinterpret_cast<const char*>(&bodyLen);
    std::string data(bodyLen_ptr, bodyLen_ptr + sizeof(bodyLen));
    data += body;
    return data;
}

void LogicSystem::Login(const std::string &user, const std::string &pwd, std::shared_ptr<Session> session)
{
    std::cout << "Login -u " << user << " -p " << pwd << std::endl;
    int ret = MysqlDao::GetInstance().Login(user, pwd);
    if (ret == 1)
    {
        std::cout << "登陆成功" << std::endl;
        nlohmann::json json;
        json["type"] = "Login";
        json["status"] = "success";
        json["profile"] = Util::FileToBase64(MysqlDao::GetInstance().GetProfileUrl(user));
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
        session->setUser(user);
    }
    else if (ret == 0)
    {
        std::cout << "用户名或密码错误" << std::endl;
        nlohmann::json json;
        json["type"] = "Login";
        json["status"] = "fail";
        json["profile"] = "";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
    else
    {
        std::cout << "登陆失败" << std::endl;
        nlohmann::json json;
        json["type"] = "Login";
        json["status"] = "error";
        json["profile"] = "";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
}

void LogicSystem::Regist(const std::string &user, const std::string &pwd, std::vector<u_char> profile, std::shared_ptr<Session> session)
{
    std::cout << "Regist -u " << user << " -p " << pwd << std::endl;
    std::string profileUrl = "./assest/profile/" + user;
    if (!std::filesystem::exists(profileUrl))
    {
        if (!Util::WriteToFile(profileUrl, profile))
        {
            std::cout << "头像写入失败" << std::endl;
        }
    }

    int ret = MysqlDao::GetInstance().RegUser(user, pwd, profileUrl);
    if (ret == 1)
    {
        std::cout << "注册成功" << std::endl;
        nlohmann::json json;
        json["type"] = "Regist";
        json["status"] = "success";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
    else if (ret == 0)
    {
        std::cout << "用户名已存在" << std::endl;
        nlohmann::json json;
        json["type"] = "Regist";
        json["status"] = "fail";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
    else
    {
        std::cout << "注册失败" << std::endl;
        nlohmann::json json;
        json["type"] = "Regist";
        json["status"] = "error";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
}

void LogicSystem::FindUser(const std::string &user, std::shared_ptr<Session> session)
{
    std::cout << "FindUser -u " << user << std::endl;
    int ret = MysqlDao::GetInstance().FindUser(user);
    if (ret == 1)
    {
        std::cout << "查找成功" << std::endl;
        nlohmann::json json;
        json["type"] = "FindUser";
        json["status"] = "success";
        json["profile"] = Util::FileToBase64(MysqlDao::GetInstance().GetProfileUrl(user));
        int isFriend = session->curUser() == user ? 1 : MysqlDao::GetInstance().IsFriend(session->curUser(), user);
        if (isFriend == 1) 
        {
            json["isFriend"] = "1";
        }
        else 
        {
            json["isFriend"] = "0";
        }
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
    else if (ret == 0)
    {
        std::cout << "未找到" << std::endl;
        nlohmann::json json;
        json["type"] = "FindUser";
        json["status"] = "fail";
        json["profile"] = "";
        json["isFriend"] = "0";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
    else
    {
        std::cout << "查找失败" << std::endl;
        nlohmann::json json;
        json["type"] = "FindUser";
        json["status"] = "error";
        json["profile"] = "";
        json["isFriend"] = "0";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
}

void LogicSystem::ApplyFriend(const std::string &user, const std::string &leave, std::shared_ptr<Session> session)
{
    const std::string &apply_user = session->curUser();
    const std::string &accept_user = user;

    std::cout << "ApplyFriend: " << apply_user << " to " << accept_user << " leave: " << leave << std::endl;

    // 执行过程
    int ret = MysqlDao::GetInstance().ApplyFriend(apply_user, accept_user, leave);
    std::cout << ret << std::endl;
    if (ret == 1)
    {
        std::cout << "申请成功" << std::endl;
        nlohmann::json json;
        json["type"] = "ApplyFriendResponse";
        json["status"] = "success";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
        
        // 被申请方在线（发送请求消息）
        if (Server::__instance->isOnline(accept_user))
        {
            m_msgs.emplace(std::bind(&LogicSystem::ApplyFriendRequest, this, apply_user, leave, Server::__instance->getSession(accept_user)));
            cv.notify_one();
        }
    }
    else if (ret == 2)
    {
        std::cout << "用户不存在" << std::endl;
        nlohmann::json json;
        json["type"] = "ApplyFriendResponse";
        json["status"] = "fail";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
    else if (ret == 3)
    {
        std::cout << "重复申请" << std::endl;
        nlohmann::json json;
        json["type"] = "ApplyFriendResponse";
        json["status"] = "repeat";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
    else
    {
        std::cout << "申请失败" << std::endl;
        nlohmann::json json;
        json["type"] = "ApplyFriendResponse";
        json["status"] = "error";
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
}

void LogicSystem::ApplyFriendRequest(const std::string &apply_user, const std::string &leave, std::shared_ptr<Session> session)
{
    std::cout << "send apply friend request..." << std::endl;
    nlohmann::json json;
    json["type"] = "ApplyFriendRequest";
    json["user"] = apply_user;
    json["leave"] = leave;
    json["profile"] = Util::FileToBase64(MysqlDao::GetInstance().GetProfileUrl(apply_user));
    const std::string &json_str = json.dump();
    session->write(toTcpData(json_str));
}

void LogicSystem::RecvApplyFriendRequest(const std::string &user, std::shared_ptr<Session> session)
{
    std::cout << "send all apply friend request..." << std::endl;
    nlohmann::json apply_arr = MysqlDao::GetInstance().GetFriendApplyList(user);
    nlohmann::json json;
    json["type"] = "RecvApplyFriendRequest";
    json["apply_arr"] = apply_arr;
    const std::string &json_str = json.dump();
    session->write(toTcpData(json_str));
}

void LogicSystem::PassFriendApply(const std::string &user, std::shared_ptr<Session> session)
{
    std::cout << "pass friend apply..." << std::endl;
    const std::string &user_accept = session->curUser();

    int ret1 = MysqlDao::GetInstance().SetFriend(user, user_accept);
    std::cout << "ret1: " << ret1 << std::endl;

    int ret2 = MysqlDao::GetInstance().SetFriend(user_accept, user);
    std::cout << "ret2: " << ret2 << std::endl;

    int ret = MysqlDao::GetInstance().ChangeFriendApplyState(user, user_accept, "pass");
    std::cout << "ret1 change state: " << ret << std::endl;

    ret = MysqlDao::GetInstance().ChangeFriendApplyState(user_accept, user, "pass");
    std::cout << "ret2 change state: " << ret << std::endl;

    // 添加好友失败或已经是好友则不添加至好友列表
    if (ret1 != 1 || ret2 != 1)
    {
        return;
    }

    // 接收方添加申请方至好友至列表
    nlohmann::json json;
    json["type"] = "AddedFriend";
    json["user"] = user;
    json["profile"] = Util::FileToBase64(MysqlDao::GetInstance().GetProfileUrl(user));
    const std::string &json_str = json.dump();
    session->write(toTcpData(json_str));

    // 如果申请方在线 则添加接收方至好友至列表
    if (Server::__instance->isOnline(user))
    {
        std::shared_ptr<Session> session = Server::__instance->getSession(user);
        nlohmann::json json;
        json["type"] = "AddedFriend";
        json["user"] = user_accept;
        json["profile"] = Util::FileToBase64(MysqlDao::GetInstance().GetProfileUrl(user_accept));
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
}

void LogicSystem::RefuseFriendApply(const std::string &user, std::shared_ptr<Session> session)
{
    std::cout << "refuse friend apply..." << std::endl;
    const std::string &user_accept = session->curUser();

    int ret = MysqlDao::GetInstance().ChangeFriendApplyState(user, user_accept, "refuse");
    std::cout << "ret change state: " << ret << std::endl;
}

void LogicSystem::RecvFriends(const std::string &user, std::shared_ptr<Session> session)
{
    std::cout << "send all friends..." << std::endl;
    nlohmann::json friend_arr = MysqlDao::GetInstance().GetFriends(user);
    nlohmann::json json;
    json["type"] = "RecvFriends";
    json["friend_arr"] = friend_arr;
    const std::string &json_str = json.dump();
    session->write(toTcpData(json_str));
}

void LogicSystem::ChangeProfile(std::vector<u_char> profile, std::shared_ptr<Session> session)
{
    std::cout << "change user profile..." << std::endl;
    const std::string &profileUrl = MysqlDao::GetInstance().GetProfileUrl(session->curUser());
    bool ret = Util::WriteToFile(profileUrl, profile);
    if (!ret)
    {
        std::cout << "头像修改失败" << std::endl;
    }
}

void LogicSystem::RemoveFriendApply(const std::string &user, std::shared_ptr<Session> session)
{
    std::cout << "remove friend apply" << std::endl;
    int ret = MysqlDao::GetInstance().RemoveFriendApply(user, session->curUser());
    if (ret != 1)
    {
        std::cout << "删除好友申请记录失败" << std::endl;
    }
}

void LogicSystem::RemoveFriend(const std::string &user, std::shared_ptr<Session> session)
{
    std::cout << "remove friend" << std::endl;
    const std::string &userA = session->curUser();
    int ret = MysqlDao::GetInstance().RemoveFriend(user, userA);
    if (ret != 1)
    {
        std::cout << "删除好友失败1:" << ret << std::endl;
    }
    ret = MysqlDao::GetInstance().RemoveFriend(userA, user);
    if (ret != 1)
    {
        std::cout << "删除好友失败2:" << ret << std::endl;
    }
    
    // 删除方删除列表好友
    nlohmann::json json;
    json["type"] = "RemoveFriend";
    json["user"] = user;
    const std::string &json_str = json.dump();
    session->write(toTcpData(json_str));

    // 如果被删除方在线 则删除好友
    if (Server::__instance->isOnline(user))
    {
        std::cout << "对方在线" << std::endl;
        std::shared_ptr<Session> session = Server::__instance->getSession(user);
        nlohmann::json json;
        json["type"] = "RemoveFriend";
        json["user"] = userA;
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
}

void LogicSystem::SendMsg(const std::string &msgId, const std::string &user, const std::string &time, const std::string &msg, bool file_msg, std::shared_ptr<Session> session)
{
    const std::string &userA = session->curUser();
    const std::string &userB = user;
    std::cout << "send message: " << userA << " -> " << userB << std::endl;

    if (!MysqlDao::GetInstance().IsFriend(userB, userA))
    {
        nlohmann::json json;
        json["type"] = "SendMsgFail";
        json["id"] = msgId;
        return;
    }

    nlohmann::json json;
    json["type"] = "RecvMsg";
    json["user"] = userA;
    json["time"] = time;
    json["msg"] = msg;
    json["file_msg"] = file_msg;
    
    // 接收方在线
    if (Server::__instance->isOnline(userB))
    {
        const std::string &json_str = json.dump();
        Server::__instance->getSession(userB)->write(toTcpData(json_str));
    }
    // 接收方不在线
    else
    {
        std::cout << userB << "不在线" << std::endl;
        Server::__instance->addWattingMsg(userB, json);
    }
}

void LogicSystem::LoadWaitMsg(std::shared_ptr<Session> session)
{
    Server::__instance->go_online(session);
    const std::string &user = session->curUser();
    const std::vector<nlohmann::json> &msgs = Server::__instance->wattingMsgs(user);
    std::cout << "watting msgs count: " << msgs.size() << std::endl;
    for (const nlohmann::json &json : msgs)
    {
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
    }
    Server::__instance->clearWattingMsgs(user);
}

void LogicSystem::Offline(std::shared_ptr<Session> session)
{
    nlohmann::json json;
    json["type"] = "Offline";
    const std::string &json_str = json.dump();
    session->write(toTcpData(json_str));
}