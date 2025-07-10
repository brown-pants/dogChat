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
};

#endif