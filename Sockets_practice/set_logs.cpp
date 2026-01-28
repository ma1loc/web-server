# include "set_logs.hpp"
# include "sockets.hpp"

void poll_logs(int poll_stat)
{
    if (poll_stat == 0)
    {
        std::cout << "LOGS: poll timeout " << TIMEOUT << "ms" << std::endl;
    }
    else if (poll_stat > 0)
    {
        std::cout << "LOGS: poll found " << poll_stat << " ready fd(s)" << std::endl;
    }
    else
    {
        std::cerr << "LOGS: poll error" << std::endl;
    }
}
