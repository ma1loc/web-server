# include "sockets.hpp"

// 6-DAYS

socket_engine::socket_engine()
{
    /*  epoll_create()
        epoll_create created a table in the kernal level to save the socket fds

    */
    epoll_fd = epoll_create(1);
    if (epoll_fd < 0)
    {
        std::cerr << "Error: epoll_create failed: " << strerror(errno) << std::endl;
        exit(1);
    }

    std::cout << "socket_engine successfully ready!" << std::endl;
}

// (DONE[*])
void socket_engine::set_client_side(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        close(fd);
        std::cerr << "[-] fcntl error on fd " << fd << ": " << strerror(errno) << std::endl;
        return;
    }

    struct epoll_event ev;

    std::memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    ev.events = EPOLLIN; // Tell me when ready to read data from FD

    /*
        epoll_fd -> table fd
        EPOLL_CTL_ADD -> Add an entry to the interest list of the epoll file descriptor
        fd -> new fd to add in the table
        ev -> fd infos
    */
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        close (fd);
        std::cerr << "[-] epoll_ctl error on fd " << fd << ": " << strerror(errno) << std::endl;
        return ;
    }
    set_fds_list(fd);
}

// ----------------------------------------------------------------------------------------------------------- //

// TODO:
// [*] create the socket
// [*] set SO_REUSEADDR
// [*] make it Non-Blocking with fcntl
// [*] bind to the correct port
// [*] listen
// [*] put it into the first slot of my poll_fds vector

// (DONE[*])
void socket_engine::set_server_side(std::string port)
{
    int serv_socketFD;

    struct epoll_event new_server_ev;
    std::memset(&new_server_ev, 0, sizeof(new_server_ev));

    // TOKNOW: handles both IPv4/v6 and support DNS resolution
    /*  TOKNOW:
        hints -> what i like to use (TCP, IPv4, etc...)
        result -> is a pointer the the same struct as the hints
            when you calling 'getaddrinfo()' system will allocate memory
            for the 'result' with infos provided in hints
    */
    struct addrinfo hints, *result;
    std::memset (&hints, 0, sizeof(hints));

    /*  TOKNOW:
        AF_INET -> IP version will be used [AF_INET(IPv4), AF_INET6(IPv6)]
        SOCK_STREAM -> socket type [UDP(SOCK_DGRAM), TCP(SOCK_STREAM)]
        AI_PASSIVE -> make the server socket listening mode
    */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = PROTOCOL_TYLE;

    /*
        getaddrinfo() is a function that done a lot of thing for you:
            parse hostnames ("localhost", "example.com")
            handle IPv4 / IPv6
            build sockaddr structures for you
        param takes:
            name: NULL -> mean's your socket will listen to all available
                interfaces
            server_port: witch port will server socket listen from
            hints: your flavors you choose (IPv4/6, TCP/UDP, etc...)
            result: apply the flavers in memory to use it in bind  
    */
    // TODO: port here's is just hardcoded, i will set it later when the parsing is ready
    if (getaddrinfo(NULL, port.c_str(), &hints, &result) != 0)
    {
        std::cerr << "getaddrinfo failed" << std::endl;
        exit(1);
    }
    // 'getaddrinfo()' at the end give us 'result' as linked-list

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
    
    // AF_INET(IPv4) | SOCK_STREAM(TCP) | SOCKTYPE->0(based on AF_INET + SOCK_STREAM) -> TCP/IPv4
    serv_socketFD = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_socketFD < 0)
    {
        std::cerr << "Error: failed open server FD\n" << errno << std::endl;
        freeaddrinfo(result);
        std::exit(1);
    }
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    new_server_ev.data.fd = serv_socketFD;
    new_server_ev.events = EPOLLIN;
    
    // TODO: setsockopt();
    /*
        setsockopt with SO_REUSEADDR opetion
        setup opetion to a serv_socketFD
        TOKNOW:
            kernal give a defult TIME_WAIT after port has ben used free
            the statement ignore that
        SOL_SOCKET -> socket_level -> tell the kernal opetions will set in the socket level

        >>> NEED TO KNOW:
        OSI Layer 7 — Application (http)
        OSI Layer 6 — Presentation (extansion, compres ,encripte(plaintext))
        OSI Layer 5 — Session (????)
        -------------------------
        Sockets API  ← (interface / boundary)
        -------------------------
        OSI Layer 4 — Transport (TCP / UDP)
        OSI Layer 3 — Network (IP)
        OSI Layer 2 — Data Link
        OSI Layer 1 — Physical

        Sockets here is the way of the Application layer(layer 7, 6, 5) will communicate
        with the next transport layer
    */

    // TEST: whithout SO_REUSEADDR -> Address already in use.
    int opt = 1;
    if (setsockopt(serv_socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        exit(1);
    }
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    // TODO: fcntl():
    /*
        Blocking and Non-blocking Sockets
        sockets creation use Blocking sockets by defult, each socket has its own kernel buffer
        Blocking vs non-blocking is a socket behavior (mode).
        Blocking sockets:
            With blocking sockets, the program can block while serving one client, and the other clients will wait.
        non-Blocking sockets:
            With non-blocking sockets, each client is served a little at a time.
        fcntl() -> manipulate file descriptor 
    */

    // TEST: without non-blocking
    // F_SETFL -> tell the kernal set the change to the 'serv_socketFD' status flag
    int fcntl_stat = fcntl(serv_socketFD, F_SETFL, O_NONBLOCK);
    if (fcntl_stat < 0)
    {
        std::cerr << "Error: fcntl failed: " << strerror(errno) << std::endl;
        freeaddrinfo(result);
        std::exit(1);
    }
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    // TODO: bind()
    // SYNTAX -> int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
    // TOKNOW: bind() assigns a (IP + port) to a socket 'serv_socketFD'
    //      IP(interface)
    //      PORT(used to listen)
   
    // >>>>>>>>>>>>>>>>>>>>> new bind() <<<<<<<<<<<<<<<<<<<<<<<<
    int bind_stat = bind(serv_socketFD, result->ai_addr, result->ai_addrlen);
    if (bind_stat < 0)
    {
        std::cerr << ">>> Error: failed to bind serv_socketFD: " << strerror(errno) << std::endl;
        freeaddrinfo(result);
        close (serv_socketFD);
        std::exit(1);
    }


    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // TODO: listen()
    /*
        SYNTAX -> int listen(int serv_socketFD, int backlog);
        listen() -> tells the kernel -> “This socket is a server socket
            Start accepting incoming connections"
        - Switches the socket into listening mode
        - Creates a queue for incoming connection requests
        backlog -> the size (limit) of the connection waiting queue.
    */
    int listen_stat = listen(serv_socketFD, QUEUE_LIMIT);
    if (listen_stat < 0)
    {
        std::cerr << "Error: failed to listen on serv_socketFD fd\n" << errno << std::endl;
        std::exit(1);
    }

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_socketFD, &new_server_ev) < 0)
    {
        std::cerr << "Error: epoll_ctl failed: " << strerror(errno) << std::endl;
        exit(1);
    }

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    freeaddrinfo(result);   // after i set the result info to the socket, bind time to free the memory
    set_fds_list(serv_socketFD);
    set_server_side_fds(serv_socketFD);
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

// ----------------------------------------------------------------------------------------------------------- //

void socket_engine::set_fds_list(int fd)
{
    fds_list.push_back(fd);
}

void socket_engine::free_fds_list(void)
{
    for (unsigned long i = 0; i < fds_list.size(); i++)
    {
        close (fds_list.at(i));
        std::cout << ">>> free fd[" << fds_list.at(i) << "]" << std::endl;
    }
    close (epoll_fd);
}

void socket_engine::set_server_side_fds(int s_fd)
{
    server_side_fds.push_back(s_fd);
}

std::vector<int> socket_engine::get_server_side_fds(void)
{
    return (server_side_fds);
}
