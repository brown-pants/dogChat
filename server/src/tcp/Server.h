#ifndef __SERVER_H__
#define __SERVER_H__
#include "Session.h"
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <mutex>
#include <set>
#include <map>

class Server
{
public:
    static Server *__instance;
    Server(boost::asio::io_context &io_context, int port);

    void go_online(std::shared_ptr<Session> session);
    void offline(const std::string &user);
    bool isOnline(const std::string &user) const;
    std::shared_ptr<Session> getSession(const std::string &user);

    void clearWattingMsgs(const std::string &user);
    std::vector<nlohmann::json> wattingMsgs(const std::string &user) const;
    void addWattingMsg(const std::string &user, const nlohmann::json &json);

private:
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::mutex _mutex;
    std::map<std::string, std::shared_ptr<Session>> m_onlineSessions;
    std::map<std::string, std::vector<nlohmann::json>> m_waiting_msgs;
    void do_accept();
};

#endif