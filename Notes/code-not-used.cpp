/* ------------------------------------------------------------------------------------------------ */

/* TODO: process connections -> poll(), accept(), resv(), send()
    SYNTAX: int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
    accept return -> file descriptor of the accepted socket (for the accepted client socket)

    poll()
        return:
            '-1'    -> error;
            '>0'    -> fd of the client;
            '0'     -> timeout;
    
    recv()
        return:
            '-1'    -> error;
            '>0'    -> Number of bytes actually read
            '0'     -> Client closed the connection



*/

// GET (DONE[ ])
// POST (DONE[ ])
// DELETE (DONE[ ])

/*
    I/O Multiplexing -> is a problem of the server to serve multiple client

    poll() -> is a 'checklist method' that gives the kernal a list of 'struct pollfd'
        with info for every single one (fd, 'events -> is the kernal action he wait for')
        and poll wait intel the kernal see an action of the spesifect event you give
        if there's a event with fd the kernal just set the revent for you
        BUT why poll is not for a big trafic ????
    epoll() -> 

    return:
        '-1'    -> error;
        '>0'    -> how many fds are ready;
        '0'     -> timeout;
    
*/

/* ------------------------------------------------------------------------------------------------ */

/*TOKNOW:
    accept return -1 in case of fd giving have no data yet
    -1 mean's a lot in case of non-blocking, and is not an error.
    errno will solve the -1 mean's an error of just the fd not ready yet
    if errno == EAGAIN -> mean's the client_fd has no data yet in the kernal table
    if errno == EWOULDBLOCK -> same as the EAGAIN just about the 
*/

/* ------------------------------------------------------------------------------------------------ */ 

    // TOKNOW: handles both IPv4/v6 and support DNS resolution
    /*  TOKNOW:
        hints -> what i like to use (TCP, IPv4, etc...)
        result -> is a pointer the the same struct as the hints
            when you calling 'getaddrinfo()' system will allocate memory
            for the 'result' with infos provided in hints
    */
    struct addrinfo hints, *result;

/* ------------------------------------------------------------------------------------------------ */ 

    /*  TOKNOW:
        AF_INET -> IP version will be used [AF_INET(IPv4), AF_INET6(IPv6)]
        SOCK_STREAM -> socket type [UDP(SOCK_DGRAM), TCP(SOCK_STREAM)]
        AI_PASSIVE -> make the server socket listening mode
    */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = PROTOCOL_TYLE;
    // AF_INET(IPv4) | SOCK_STREAM(TCP) | SOCKTYPE->0(based on AF_INET + SOCK_STREAM) -> TCP/IPv4


/* ------------------------------------------------------------------------------------------------ */ 

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
    // TODO-LIST: port here's is just hardcoded, i will set it later when the parsing is ready
    if (getaddrinfo(NULL, port.c_str(), &hints, &result) != 0)

/* ------------------------------------------------------------------------------------------------ */ 

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

/* ------------------------------------------------------------------------------------------------ */ 

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

/* ------------------------------------------------------------------------------------------------ */ 

// TODO: bind()
    // SYNTAX -> int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
    // TOKNOW: bind() assigns a (IP + port) to a socket 'serv_socketFD'
    //      IP(interface)
    //      PORT(used to listen)
   
    // >>>>>>>>>>>>>>>>>>>>> new bind() <<<<<<<<<<<<<<<<<<<<<<<<
    int bind_stat = bind(serv_socketFD, result->ai_addr, result->ai_addrlen);

/* ------------------------------------------------------------------------------------------------ */ 

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

/* ------------------------------------------------------------------------------------------------ */ 

    /*
        epoll_fd -> table fd
        EPOLL_CTL_ADD -> Add an entry to the interest list of the epoll file descriptor
        fd -> new fd to add in the table
        ev -> fd infos
    */
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) 

/* ------------------------------------------------------------------------------------------------ */ 

void epoll_logs(int epoll_stat)
{
    if (epoll_stat == 0)
    {
        std::cout << "[LOG] poll timeout " << TIMEOUT << "ms " << strerror(errno) << std::endl;
    }
    else if (epoll_stat > 0)
    {
        std::cout << "[LOG] poll found " << epoll_stat << " ready fd(s): "  << strerror(errno) << std::endl;
    }
    else
    {
        std::cerr << "[LOG] poll error: "  << strerror(errno) << std::endl;
    }
}
