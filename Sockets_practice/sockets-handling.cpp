# include "sockets.hpp"

int main()
{
    socket_engine s_engine;
    
    // NOTE: server_side will set based on the number of server block is there
    s_engine.set_server_side(8080);
    s_engine.set_server_side(9090);
    
    // NOTE: client_side will not set like server_side, becouse there's no client yet
    s_engine.set_client_side(0);
    s_engine.set_client_side(1);
    s_engine.set_client_side(2);
    
    s_engine.process_connections();

    return (0);
}