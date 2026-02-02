# include "../socket_engine.hpp"

// (DONE[*])
void socket_engine::init_server_side(std::string port, std::string host)
{

    int serv_socketFD;
    struct epoll_event new_server_ev;
    struct addrinfo hints, *result;

    std::memset(&new_server_ev, 0, sizeof(new_server_ev));
    std::memset (&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = PROTOCOL_TYLE;

    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &result) != 0)
    {
        // TODO-CHECK: LEAKKKKKKKKKKKKKKKKKKKKK
        std::cerr << "[-] Error: 'getaddrinfo' failed: " << strerror(errno) << std::endl;
        exit(1);
    }

    serv_socketFD = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_socketFD < 0)
    {
        std::cerr << "[-] Error: 'socket' failed: " << strerror(errno) << std::endl;
        freeaddrinfo(result);
        std::exit(1);
    }

    new_server_ev.data.fd = serv_socketFD;
    new_server_ev.events = EPOLLIN;

    int opt = 1;
    if (setsockopt(serv_socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[-] Error: 'setsockopt' failed: " << strerror(errno) << std::endl;
        close(serv_socketFD);
        std::exit(1);
    }
    if (fcntl(serv_socketFD, F_SETFL, O_NONBLOCK) < 0)
    {
        std::cerr << "[-] Error: 'fcntl' failed: " << strerror(errno) << std::endl;
        freeaddrinfo(result);
        close(serv_socketFD);
        std::exit(1);
    }
    if (bind(serv_socketFD, result->ai_addr, result->ai_addrlen) < 0)
    {
        std::cerr << "[-] Error: 'bind' failed: " << strerror(errno) << std::endl;
        freeaddrinfo(result);
        close (serv_socketFD);
        std::exit(1);
    }
    if (listen(serv_socketFD, QUEUE_LIMIT) < 0)
    {
        std::cerr << "[-] Error: listen failed: " << strerror(errno) << std::endl;
        close(serv_socketFD);
        std::exit(1);
    }
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_socketFD, &new_server_ev) < 0)
    {
        std::cerr << "[-] Error: 'epoll_ctl' failed: " << strerror(errno) << std::endl;
        close(serv_socketFD);
        std::exit(1);
    }

    freeaddrinfo(result);   // after i set the result info to the socket, bind time to free the memory
    set_fds_list(serv_socketFD);
    set_server_side_fds(serv_socketFD);
}
