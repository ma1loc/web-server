# include "sockets.hpp"

socket_engine::socket_engine(int port)
{

    std::memset(&server_addr_in, 0, sizeof(server_addr_in));
    std::memset(&client_addr_in, 0, sizeof(client_addr_in));
    serv_fds_count = 0;

    // >>> TODO: init server side with dynamic values later
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    server_addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::cout << "socket_engine successfully ready!" << std::endl;
}

void socket_engine::set_client_fd(int fd)
{
    struct pollfd new_clientFD;
    std::memset(&new_clientFD, 0, sizeof(new_clientFD));

    new_clientFD.fd = fd;
    new_clientFD.events = POLLIN; // Tell me when ready to read data from FD
    // new_clientFD.revents is set by the kernal

    clientFD.push_back(new_clientFD);
}

// ------------------------------------------------------------------------- //

// TODO:
// [*] Did I create the socket?
// [ ] Did I set SO_REUSEADDR?
// [ ] Did I make it Non-Blocking with fcntl?
// [ ] Did I bind to the correct port?
// [ ] Did I listen?
// [ ] Did I put it into the first slot of my poll_fds vector?

// >>> TODO: setup server side listening fd
void socket_engine::set_server_side(int port)
{
    int serv_socketFD;
    
    // AF_INET(IPv4)  -  SOCK_STREAM(TCP) - 0(based on AF_INET + SOCK_STREAM) -> TCP/IPv4
    serv_socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (!serv_socketFD)
    {
        std::cerr << "Error: Can't open server fd \n" << errno << std::endl;
        std::exit(1);
    }
    // TODO: setsockopt with SO_REUSEADDR opetion
    // >>> setup opetion to a serv_socketFD
    int optval = 1;
    // TEST: whithout SO_REUSEADDR -> Address already in use.
    // TOKNOW: kernal give a defult time after it's free to use, the statement ignore that
    setsockopt(serv_socketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // bind

    std::cout << "socker_made return status -> " << serv_socketFD << std::endl;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

int socket_engine::get_clint_fd(int index) const {
    if (index >= clientFD.size())
        return (clientFD.at(index).fd);
    return (-1);
}

int socket_engine::get_server_fd(int index) const {
        if (index < serv_fds_count)
    return (clientFD.at(index).fd);
}
