#include "Session.h"
#include "LogicSystem.h"
#include <iostream>

Session::Session(boost::asio::io_context &io_context)
    : m_socket(io_context), is_recv_head(true)
{
    
}

void Session::start()
{
    do_read(0);
}

void Session::close()
{
    m_socket.close();
}

void Session::do_read(int offset)
{
    std::shared_ptr<Session> self = shared_from_this();
    m_socket.async_read_some(boost::asio::buffer(m_msg.data() + offset, Message::max_data_len - offset), [self, this](const boost::system::error_code& ec, std::size_t  bytesTransfered){
        if (!ec)
        {
            m_msg.setLen(m_msg.curLen() + bytesTransfered);
            for(;;)
            {  
                if (is_recv_head)
                {
                    // read head
                    if (m_msg.curLen() < Message::head_len)
                    {
                        do_read(m_msg.curLen());
                        return;
                    }
                    is_recv_head = false;
                    m_msg.updateHead();
                }
                // read body
                if (m_msg.curLen() < m_msg.bodyLen() + Message::head_len)
                {// 不够
                    do_read(m_msg.curLen());
                    return;
                }
                std::string msgBody(m_msg.data() + Message::head_len, m_msg.bodyLen());
                //std::cout << "msg:" << " body len: " << m_msg.bodyLen() << " body: " << msgBody << std::endl;

                // 添加消息
                LogicSystem::GetInstance().pushMsg(shared_from_this(), msgBody);

                // 下一个数据
                memcpy(m_msg.data(), m_msg.data() + Message::head_len + m_msg.bodyLen(), Message::max_data_len - Message::head_len - m_msg.bodyLen());
                m_msg.setLen(m_msg.curLen() - Message::head_len - m_msg.bodyLen());
                is_recv_head = true;
            }
        }
        else
        {
            std::cout << "session read failed, error is " << ec.what() << std::endl;
        }  
    });
}

void Session::do_write()
{
    std::shared_ptr<Session> self = shared_from_this();
    std::lock_guard<std::mutex> lock(mtx);
    
    if (sendQueue.empty()) return;
    const std::string &msg = sendQueue.front();

    boost::asio::async_write(m_socket, boost::asio::buffer(msg), [self, this](boost::system::error_code ec, std::size_t length) {
        if (ec)
        {
            std::cout << "session sent failed, error is " << ec.what() << std::endl;
        }
        std::lock_guard<std::mutex> lock(mtx);
        sendQueue.pop();
        if (!sendQueue.empty())
        {
            do_write();
        }
    });
}

void Session::write(const std::string &msg)
{
    mtx.lock();
    sendQueue.push(msg);
    mtx.unlock();
    do_write();
}