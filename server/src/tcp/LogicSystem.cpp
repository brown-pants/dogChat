#include "LogicSystem.h"
#include "../dao/MySQLDao.h"
#include "../util/Util.h"
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
            std::vector<u_char> binary = Util::Base64ToBinary(json["profile"]);
            // push msg
            m_msgs.emplace(std::bind(&LogicSystem::Regist, this, json["user"], json["password"], binary, session));
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
        json["profile"] = Util::FileToBase64("./assest/profile/" + user);
        const std::string &json_str = json.dump();
        session->write(toTcpData(json_str));
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
        std::cout << "登陆失败" << std::endl;nlohmann::json json;
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