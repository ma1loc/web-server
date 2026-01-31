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
# include <cerrno>
# include <cstdlib>
# include <fcntl.h>
# include <netdb.h>
# include <sys/epoll.h>
# include <map>

# include <iostream>

# define TIMEOUT 1000
# define QUEUE_LIMIT 128
# define BUFFER_SIZE 1024 // -> size in bytes
# define PROTOCOL_TYLE 0

# define MAX_EVENTS 64

class socket_engine {
    private:
        int epoll_fd;   // ID for the table
        struct epoll_event events[MAX_EVENTS];
        std::vector<int> server_side_fds;   // >>> backup for the server socket fds
        std::vector<int> fds_list;  // >>> backup for all the fds used to free them in case of SIGINT
        std::map<int, std::string> package_statement; // >>> raw data stored in 'package_statement' based on it's fd

    public:
        socket_engine();
        void    set_server_side(std::string port);
        void    set_client_side(int fd);
        void    process_connections(void);

        void    set_fds_list(int fd);
        void    free_fds_list(void);

        void    set_server_side_fds(int s_fd);
        std::vector<int>    get_server_side_fds(void);
};


# endif