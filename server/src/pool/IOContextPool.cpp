#include "IOContextPool.h"

IOContextPool &IOContextPool::GetInstance()
{
    static IOContextPool instance;
    return instance;
}

IOContextPool::IOContextPool(size_t size)
    : m_ioContexts(size), m_works(size), nextIOContext(0)
{
    for (int i = 0; i < size; i ++)
    {
        m_works.emplace_back(std::make_unique<Work>(m_ioContexts[i]));
    }

    for (int i = 0; i < size; i ++)
    {
        m_threads.emplace_back([this, i](){
            m_ioContexts[i].run();
        });
    }
}

IOContextPool::~IOContextPool()
{
    
}

IOContextPool::IOContext &IOContextPool::getIOContext()
{
    IOContext &ioContext = m_ioContexts[nextIOContext ++];
    if (nextIOContext == m_ioContexts.size())
    {
        nextIOContext = 0;
    }
    return ioContext;
}

void IOContextPool::stop()
{
    for (int i = 0; i < m_works.size(); i ++)
    {
        m_works[i]->get_io_context().stop();
        m_works[i].reset();
    }

    for (std::thread &th : m_threads)
    {
        th.join();
    }
}