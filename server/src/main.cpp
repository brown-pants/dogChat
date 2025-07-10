#include "config/Config.h"
#include "tcp/Server.h"
#include "tcp/LogicSystem.h"
#include "pool/IOContextPool.h"

int main(int argc, char *argv[])
{
    if (!Config::Init()) return -1;

    try 
    {
        IOContextPool &pool = IOContextPool::GetInstance();
        LogicSystem &logic_sys = LogicSystem::GetInstance();
        
        boost::asio::io_context  io_context;
        Server server(io_context, std::stoi(Config::server_port()));
        io_context.run();
        logic_sys.dealMsgs();

        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&io_context, &logic_sys, &pool](auto, auto) {
            io_context.stop();
            pool.stop();
            logic_sys.stop();
        });
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}