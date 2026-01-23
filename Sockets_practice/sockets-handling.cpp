# include "sockets.hpp"

int main()
{
    socket_engine s_engine(8080);

    s_engine.set_client_fd(999);
    s_engine.set_client_fd(1);
    s_engine.set_client_fd(2);
    s_engine.set_client_fd(3);

    s_engine.set_server_side();

    std::cout << "server fd -> " << s_engine.get_server_fd() << std::endl;

    return (0);
}