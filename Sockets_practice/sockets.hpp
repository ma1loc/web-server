# ifndef SOCKETS_HPP
# define SOCKETS_HPP

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <cstring>
# include <stdint.h>
# include <vector>
# include <iostream>
# include <poll.h>
# include <cerrno>
# include <cstdlib>

# define TIMEOUT 1000

class socket_engine {
    private:
        std::vector<struct pollfd> poll_fds;
        unsigned short int serv_fds_count;

    public:
        socket_engine();

        void    set_client_side(int fd);
        void    set_server_side(int port);
};

/*
    i have to handle the server first in every single port is provided and stor-them in std::vector<struct pollfd> poll_fds;
    info set like this of the server:
        server_addr_in.sin_family = AF_INET;
        server_addr_in.sin_port = htons(port);
        server_addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
    the client after will be handeld by the set_client_size:
        new_clientFD.fd = fd;
        new_clientFD.events = POLLIN; // Tell me when ready to read data from FD
        // new_clientFD.revents is set by the kernal
*/ 

# endif