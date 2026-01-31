# include "set_logs.hpp"
# include "sockets.hpp"

void epoll_logs(int epoll_stat)
{
    if (epoll_stat == 0)
    {
        std::cout << "[LOG] poll timeout " << TIMEOUT << "ms " << strerror(errno) << std::endl;
    }
    else if (epoll_stat > 0)
    {
        std::cout << "[LOG] poll found " << epoll_stat << " ready fd(s): "  << strerror(errno) << std::endl;
    }
    else
    {
        std::cerr << "[LOG] poll error: "  << strerror(errno) << std::endl;
    }
}
