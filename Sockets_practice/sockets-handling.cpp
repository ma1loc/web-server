# include "sockets.hpp"

int main()
{
    socket_engine s_engine;
    
    s_engine.set_server_side(8080);

    s_engine.set_client_side(999);
    s_engine.set_client_side(1);
    s_engine.set_client_side(2);
    s_engine.set_client_side(3);


    // std::cout << "server fd -> " << s_engine.get_server_fd() << std::endl;

    return (0);
}