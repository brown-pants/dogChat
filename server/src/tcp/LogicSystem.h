#ifndef __LOGICSYSTEM_H__
#define __LOGICSYSTEM_H__
#include <string>
#include <queue>
#include <nlohmann/json.hpp>
#include <mutex>
#include <thread>
#include <memory>
#include <condition_variable>
#include "Session.h"

class LogicSystem
{
public:
    static LogicSystem &GetInstance();
    void pushMsg(std::shared_ptr<Session> session, const std::string &json_str);
    void dealMsgs();
    void stop();

    void Offline(std::shared_ptr<Session> session);

private:
    LogicSystem();
    std::queue<std::function<void()>> m_msgs;
    bool isStop;
    std::mutex mtx;
    std::thread thread;
    std::condition_variable cv;
    std::string toTcpData(const std::string body);
    void Login(const std::string &user, const std::string &pwd, std::shared_ptr<Session> session);
    void Regist(const std::string &user, const std::string &pwd, std::vector<u_char> profile, std::shared_ptr<Session> session);
    void FindUser(const std::string &user, std::shared_ptr<Session> session);
    void ApplyFriend(const std::string &user, const std::string &leave, std::shared_ptr<Session> session);
    void ApplyFriendRequest(const std::string &apply_user, const std::string &leave, std::shared_ptr<Session> session);
    void RecvApplyFriendRequest(const std::string &user, std::shared_ptr<Session> session);
    void RecvFriends(const std::string &user, std::shared_ptr<Session> session);
    void PassFriendApply(const std::string &user, std::shared_ptr<Session> session);
    void RefuseFriendApply(const std::string &user, std::shared_ptr<Session> session);
    void ChangeProfile(std::vector<u_char> profile, std::shared_ptr<Session> session);
    void RemoveFriendApply(const std::string &user, std::shared_ptr<Session> session);
    void RemoveFriend(const std::string &user, std::shared_ptr<Session> session);
    void SendMsg(const std::string &msgId, const std::string &user, const std::string &time, const std::string &msg, bool file_msg, std::shared_ptr<Session> session);
    void LoadWaitMsg(std::shared_ptr<Session> session);
};

#endif