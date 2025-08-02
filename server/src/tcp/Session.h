#ifndef __SESSION_H__
#define __SESSION_H__
#include "Message.h"
#include <boost/asio.hpp>
#include <memory>
#include <queue>

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context &io_context);
    void start();
    void close();
    boost::asio::ip::tcp::socket &getSocket() { return m_socket; }
    void write(const std::string &msg);
    void setUser(const std::string &user);
    std::string curUser() const;

private:
    Message m_msg;
    boost::asio::ip::tcp::socket m_socket;
    bool is_recv_head;
    void do_read(int offset);
    void do_write();
    std::queue<std::string> sendQueue;
    std::mutex mtx;
    std::string user;
    bool is_writing;
};

#endif