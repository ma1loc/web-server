# include "./infrastructure_init/socket_engine.hpp"
# include <csignal>

// 8-DAYS

socket_engine s_engine;

void signal_handler(int sig) {
    std::cout << "Interrupt handle " << sig << std::endl;
    s_engine.free_fds_list();
    exit(sig);
}

int main()
{
    signal(SIGINT, signal_handler);
    
    // NOTE: server_side will set based on
    //      the number of server block is there
    s_engine.init_server_side("8080", "localhos");
    s_engine.init_server_side("9090", "localhost");

    s_engine.process_connections(); 

    return (0);
}