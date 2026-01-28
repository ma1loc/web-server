# include "sockets.hpp"
# include <csignal>

socket_engine s_engine;

void signal_handler(int sig) {
    std::cout << "Interrupt handle " << sig << std::endl;
    s_engine.free_poll_fds();
    exit(sig);
}

int main()
{
    signal(SIGINT, signal_handler);
    
    // NOTE: server_side will set based on the number of server block is there
    s_engine.set_server_side("8080");
    s_engine.set_server_side("9090");
    
    s_engine.process_connections();

    return (0);
}