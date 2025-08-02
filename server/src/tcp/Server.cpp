#include "Server.h"
#include "../util/Util.h"
#include "../pool/IOContextPool.h"
#include "LogicSystem.h"
#include <iostream>

Server *Server::__instance = nullptr;

Server::Server(boost::asio::io_context &io_context, int port)
    : m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
{
    std::cout << "服务器已启动，监听端口号：" << port << std::endl;
    __instance = this;
    do_accept();
}

void Server::do_accept()
{
    boost::asio::io_context &io_context = IOContextPool::GetInstance().getIOContext();
    std::shared_ptr<Session> new_session = std::make_shared<Session>(io_context);
    m_acceptor.async_accept(new_session->getSocket(), [this, new_session](const boost::system::error_code& error){
        if (!error) 
        {
            boost::asio::ip::tcp::endpoint session_endpoint = new_session->getSocket().remote_endpoint();
            std::cout << "new accept: " << session_endpoint.address() << ":" << session_endpoint.port() << std::endl;
            new_session->start();
            std::lock_guard<std::mutex> lock(_mutex);
        }
        else 
        {
            std::cout << "session accept failed, error is " << error.what() << std::endl;
        }
        do_accept();
    });
}

void Server::go_online(std::shared_ptr<Session> session)
{
    auto iter = m_onlineSessions.find(session->curUser());
    if (iter != m_onlineSessions.end())
    {// 挤下线
        std::cout << session->curUser() << " 被挤下线" << std::endl;
        LogicSystem::GetInstance().Offline(iter->second);
        m_onlineSessions.erase(iter);
    }
    m_onlineSessions.insert(std::pair<std::string, std::shared_ptr<Session>>(session->curUser(), session));
    std::cout << session->curUser() << " 已上线" << std::endl;
}

void Server::offline(const std::string &user)
{
    if (m_onlineSessions.erase(user))
    {
        std::cout << user << " 已下线" << std::endl;
    }
}

bool Server::isOnline(const std::string &user) const
{
    return m_onlineSessions.find(user) == m_onlineSessions.end() ? false : true;
}

std::shared_ptr<Session> Server::getSession(const std::string &user)
{
    auto iter = m_onlineSessions.find(user);
    if (iter == m_onlineSessions.end())
    {
        return nullptr;
    }
    return iter->second;
}

void Server::clearWattingMsgs(const std::string &user)
{
    auto iter = m_waiting_msgs.find(user);
    if (iter != m_waiting_msgs.end())
    {
        m_waiting_msgs.erase(iter);
    }
}

std::vector<nlohmann::json> Server::wattingMsgs(const std::string &user) const
{
    auto iter = m_waiting_msgs.find(user);
    if (iter == m_waiting_msgs.end())
    {
        std::vector<nlohmann::json> vec(0);
        return vec;
    }
    return iter->second;
}

void Server::addWattingMsg(const std::string &user, const nlohmann::json &json)
{
    auto iter = m_waiting_msgs.find(user);
    if (iter == m_waiting_msgs.end())
    {
        m_waiting_msgs.insert(std::pair<std::string, std::vector<nlohmann::json>>(user, { json }));
    }
    else
    {
        iter->second.push_back(json);
    }
}