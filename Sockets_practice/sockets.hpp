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
# include <fcntl.h>
# include <map>
# include <netdb.h>

# define TIMEOUT 1000
# define QUEUE_LIMIT 128
# define BUFFER_SIZE 1024 // -> size in bytes???
# define PROTOCOL_TYLE 0


class socket_engine {
    private:
        std::vector<struct pollfd> poll_fds;
        unsigned short int serv_fds_count;
        unsigned int pool_fds_len;
        bool is_running;
    public:
        socket_engine();
        void    set_server_side(std::string port);
        void    set_client_side(unsigned short int fd);
        void    process_connections(void);

        // std::vector<struct pollfd> &get_poll_fds(void);

        unsigned short int  get_serv_fds_count(void) const;
        unsigned int        get_pool_fds_len(void) const;
};


# endif