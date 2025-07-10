#ifndef __SERVER_H__
#define __SERVER_H__
#include <boost/asio.hpp>
#include <mutex>

class Server
{
public:
    Server(boost::asio::io_context &io_context, int port);
private:
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::mutex _mutex;
    void do_accept();
};

#endif