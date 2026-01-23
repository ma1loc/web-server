struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
};


// - For Sockets info
struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};

// - For bind()
struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
};

struct pollfd {
    int fd;         // the socket descriptor
    short events;   // bitmap of events we're interested in
    short revents;  // on return, bitmap of events that occurred
};

// ------------------------------------------------------------ //


// will init s_addr with "inet_addr()"
struct in_addr {
    uint32_t s_addr;
};

struct sockaddr_in {
    /*
        sin_family => take type of IP version will used
            - AF_INET (IPv4)
            - AF_INET6 (IPv6)
    */
    short int          sin_family;
    
    /*
        sin_port => hold the port number
        will use htons() function for precessing order
        - netwok use big-endin, but CPU's arch it can be little-endin
            that why use it to enforce using big-endian 
    */
    unsigned short int sin_port;

    /*
        socker 
    */ 
    struct in_addr     sin_addr;
    unsigned char      sin_zero[8];
};


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//  0  1  2  3  4  5  indx
// [1. 2. 3] 4  5  6  fds
int socket_engine::get_clint_fd(int index) const {

    if (index <= (serv_fds_count -1) && index >= clientFD.size())
        return (clientFD.at(index).fd);
    return (-1);
}

int socket_engine::get_server_fd(int index) const {
    if (index < serv_fds_count)
    return (clientFD.at(index).fd);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

struct sockaddr_in server_addr_in;
struct sockaddr_in client_addr_in;

std::memset(&server_addr_in, 0, sizeof(server_addr_in));
std::memset(&client_addr_in, 0, sizeof(client_addr_in));


// >>> TODO: init server side with dynamic values later
server_addr_in.sin_family = AF_INET;
server_addr_in.sin_port = htons(port);
server_addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
