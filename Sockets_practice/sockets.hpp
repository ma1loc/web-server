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

        std::vector<struct pollfd> clientFD;
        unsigned short int serv_fds_count;
        struct sockaddr_in server_addr_in;
        struct sockaddr_in client_addr_in;
    public:
        socket_engine(int port);

        // setters
        void    set_client_fd(int fd);
        void    set_server_side(int port);

        // getters
        int     get_clint_fd(int index) const;
        int     get_server_fd(int index) const;
};

# endif