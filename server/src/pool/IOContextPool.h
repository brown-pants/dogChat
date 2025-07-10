#ifndef __IOCONTEXTPOOL_H__
#define __IOCONTEXTPOOL_H__
#include <boost/asio.hpp>

class IOContextPool
{
public:
    using IOContext = boost::asio::io_context;
    using Work = boost::asio::io_context::work;
    using WorkPtr = std::unique_ptr<Work>;

    static IOContextPool &GetInstance();
    ~IOContextPool();

    IOContext &getIOContext();
    void stop();

private:
    IOContextPool(size_t size = std::thread::hardware_concurrency());
    std::vector<IOContext> m_ioContexts;
    std::vector<WorkPtr> m_works;
    std::vector<std::thread> m_threads;
    size_t nextIOContext;
};

#endif