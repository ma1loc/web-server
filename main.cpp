# include "./infrastructure_init/socket_engine.hpp"
# include <csignal>

// 10-DAYS

socket_engine s_engine;

void signal_handler(int sig) {
    std::cout << "Interrupt handle " << sig << std::endl;
    s_engine.free_fds_list();
    exit(sig);
}

int main()
{
    signal(SIGINT, signal_handler);

    // TODO_KNOW: parsing the config file will start first
    
    // NOTE: server_side will set based on
    //      the number of server block is there
    s_engine.init_server_side("8080", "localhost"); // done[*]
    s_engine.init_server_side("9090", "localhost"); // done[*]

    s_engine.process_connections(); // done []

    return (0);
}