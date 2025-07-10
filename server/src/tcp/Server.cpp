#include "Server.h"
#include "Session.h"
#include "../util/Util.h"
#include "../pool/IOContextPool.h"
#include <iostream>

Server::Server(boost::asio::io_context &io_context, int port)
    : m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
{
    std::cout << "服务器已启动，监听端口号：" << port << std::endl;
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