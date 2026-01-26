# include "sockets.hpp"
# include <sys/socket.h>

// 3-DAYS

socket_engine::socket_engine()
{
    this->serv_fds_count = 0;
    this->pool_fds_len = 0;
    this->is_running = true;
    std::cout << "socket_engine successfully ready!" << std::endl;
}

void socket_engine::set_client_side(unsigned short int fd)
{
    struct pollfd new_client;
    std::memset(&new_client, 0, sizeof(new_client));

    new_client.fd = fd;
    new_client.events = POLLIN; // Tell me when ready to read data from FD
    // new_client.revents is set by the kernal

    poll_fds.push_back(new_client);
    pool_fds_len++;
}

// ----------------------------------------------------------------------------------------------------------- //

// TODO:
// [*] create the socket
// [*] set SO_REUSEADDR
// [*] make it Non-Blocking with fcntl
// [*] bind to the correct port
// [*] listen
// [*] put it into the first slot of my poll_fds vector

// >>> TODO: setup server side listening fd
void socket_engine::set_server_side(std::string port)
{
    int serv_socketFD;
    struct pollfd new_server;

    // TOKNOW: handles both IPv4/v6 and support DNS resolution
    /*  TOKNOW:
        hints -> what i like to use (TCP, IPv4, etc...)
        result -> is a pointer the the same struct as the hints
            when you calling 'getaddrinfo()' system will allocate memory
            for the 'result' with infos provided in hints
    */
    struct addrinfo hints, *result;
    std::memset (&hints, 0, sizeof(hints));
    std::memset (&result, 0, sizeof(result));
    std::memset(&new_server, 0, sizeof(new_server));


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
        std::cerr << "getaddrinfo filed" << std::endl;
        exit(1);
    }
    // 'getaddrinfo()' at the end give us 'result' as linked-list

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> //
    
    // AF_INET(IPv4) | SOCK_STREAM(TCP) | SOCKTYPE->0(based on AF_INET + SOCK_STREAM) -> TCP/IPv4
    serv_socketFD = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_socketFD < 0)
    {
        std::cerr << "Error: Can't open server FD\n" << errno << std::endl;
        std::exit(1);
    }
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    new_server.fd = serv_socketFD;
    new_server.events = POLLIN;
    
    // TODO: setsockopt();
    /*
        setsockopt with SO_REUSEADDR opetion
        setup opetion to a serv_socketFD
        TOKNOW:
            kernal give a defult TIME_WAIT after port has ben used free
            the statement ignore that
        SOL_SOCKET -> ???
    */
    // TEST: whithout SO_REUSEADDR -> Address already in use.
    int optval = 1; // >>> ???
    setsockopt(serv_socketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

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
        std::cerr << "Error: Can't Perform 'fcntl()' to provided FD\n" << errno << std::endl;
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
        std::cerr << "Error: field to bind serv_socketFD fd\n" << errno << std::endl;
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
        std::cerr << "Error: field to listen on serv_socketFD fd\n" << errno << std::endl;
        std::exit(1);
    }
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    freeaddrinfo(result);   // after i set the result info to the socket, bind time to free the memory
    poll_fds.push_back(new_server);
    serv_fds_count++;
    pool_fds_len++;
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

// ----------------------------------------------------------------------------------------------------------- //

unsigned short int socket_engine::get_serv_fds_count(void) const {
    return (this->serv_fds_count);
}
// ----------------------------------------------------------------------------------------------------------- //

unsigned int socket_engine::get_pool_fds_len(void) const {
    return (this->pool_fds_len);
}
// ----------------------------------------------------------------------------------------------------------- //

// std::vector<struct pollfd> &socket_engine::get_poll_fds(void) {
//     return (this->poll_fds);
// }

// ----------------------------------------------------------------------------------------------------------- //

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

// multiplexer I/O
void socket_engine::process_connections(void)
{
    int poll_stat = 0;
    std::map<int, std::string> package_statement;

    while (this->is_running) 
    {
        // poll_fds.data() -> 
        poll_stat = poll(poll_fds.data(), poll_fds.size(), 1000);
        if (poll_stat < 0)
        {
            std::cerr << "Error: poll field to watch sockets\n" << errno << std::endl;
            std::exit(1);
        }

        // TOKNOW: will check every fds in poll_fds even server or client?
        for (unsigned long i = 0; i < poll_fds.size(); i++)
        {
            if (poll_fds.at(i).revents & POLLIN)    // client
            {
                /*
                    check if it's a server or client
                        server -> accept()
                        client -> resv()
                */
                if (i < serv_fds_count) // >>> server FD
                {
                    struct sockaddr_in client_addr;
                    std::memset(&client_addr, 0, sizeof(client_addr));
                    socklen_t client_len = sizeof(client_addr);

                    // CASE: client's internet cuts out exactly at the moment they try to connect,
                    int new_fd = accept(poll_fds.at(i).fd, (struct sockaddr *)&client_addr, &client_len);
                    if (new_fd == -1)
                        continue ;
                    
                    int fcntl_stat = fcntl(new_fd, F_SETFL, O_NONBLOCK);
                    if (fcntl_stat < 0)
                    {
                        std::cerr << "Error: Can't Perform 'fcntl()' to provided FD\n" << errno << std::endl;
                        std::exit(1);
                    }
                    this->set_client_side(new_fd);
                } else {    // >>> client FD
                    // TODO: use the resv() function to get the client request
                    // here i will hae to read the request the client send() using the resv()?
                    /* 
                        TOKNOW: client request size is unknown, will use a static size buffer
                        >>> is i have to done the parsing of what i read from request here?
                        >>> to get end of the header just check four reading char -> '\r\n\r\n'
                        if you reading some thing else after the '\r\n\r\n' that a body
                        >>> NOTE: will using the std::map<int, std::string> here.
                    */

                    char buffer[BUFFER_SIZE];
                    /*
                        TOKNOW: recv last flag param is for the FD behaver, 0 -> defult behaver used
                        every single socket created has it's own kernal buffer
                        >>> the client just send a TCP segments each time and the recv take each time a sigment
                        the sigment size based on the network bandwith?
                        every TCP segments kernal recv will append to the old segments in kernal queue?
                    */

                    ssize_t bytes_received = recv(poll_fds.at(i).fd, buffer, sizeof(buffer), 0);
                    if (bytes_received <= 0)
                        std::cerr << errno << std::endl;
                    else if (bytes_received > 0) // thers's
                    {}
                    // if condition will check if there's more data to resv
                    /*
                        what i will don't know is 
                    */
                    if (bytes_received < 0) //
                }
            }
            std::cout << "say hey form for loop" << std::endl;
        }
        std::cout << "say hey form while loop" << std::endl;
    }
}

