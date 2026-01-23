# include "sockets.hpp"

socket_engine::socket_engine()
{
    serv_fds_count = 0;
    std::cout << "socket_engine successfully ready!" << std::endl;
}

void socket_engine::set_client_side(int fd)
{
    struct pollfd new_client;
    std::memset(&new_client, 0, sizeof(new_client));

    new_client.fd = fd;
    new_client.events = POLLIN; // Tell me when ready to read data from FD
    // new_client.revents is set by the kernal

    poll_fds.push_back(new_client);
}

// ------------------------------------------------------------------------- //

// TODO:
// [*] create the socket
// [ ] set SO_REUSEADDR
// [ ] make it Non-Blocking with fcntl
// [ ] bind to the correct port
// [ ] listen
// [ ] put it into the first slot of my poll_fds vector

// >>> TODO: setup server side listening fd
void socket_engine::set_server_side(int port)
{
    int serv_socketFD;
    struct pollfd new_server;
    struct sockaddr_in server_addr_in;

    std::memset(&new_server, 0, sizeof(new_server));
    std::memset(&server_addr_in, 0, sizeof(server_addr_in));
    
    // AF_INET(IPv4)  -  SOCK_STREAM(TCP) - 0(based on AF_INET + SOCK_STREAM) -> TCP/IPv4
    serv_socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_socketFD < 0)
    {
        std::cerr << "Error: Can't open server fd \n" << errno << std::endl;
        std::exit(1);
    }

    new_server.fd = serv_socketFD;
    new_server.events = POLLIN;
    
    // TODO: setsockopt with SO_REUSEADDR opetion
    // >>> setup opetion to a serv_socketFD
    int optval = 1;
    // TEST: whithout SO_REUSEADDR -> Address already in use.
    // TOKNOW: kernal give a defult time after it's free to use, the statement ignore that
    setsockopt(serv_socketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // >>> before the bind i have to set the sockaddr_in struct
    server_addr_in.sin_family = AF_INET;
    server_addr_in.sin_port = htons(port);
    server_addr_in.sin_addr.s_addr = INADDR_ANY;

    poll_fds.push_back(new_server);
    serv_fds_count++;
    
    // will done the rest later on check check the other things
    // TODO: bind()

    std::cout << "socker_made return status -> " << serv_socketFD << std::endl;
}

// ------------------------------------------------------------------------- //