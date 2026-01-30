# include "sockets.hpp"
# include "set_logs.hpp"
# include <iterator>
# include <algorithm>

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

void socket_engine::process_connections(void)
{
    std::map<int, std::string> package_statement;

    // >>> main loop will exist just in one case of signal
    while (true)
    {
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        epoll_logs(epoll_stat);

        // ------------------------------------------------------------------- //

        std::cout << "epoll return -> " << epoll_stat << std::endl;
        for (int i = 0; i < epoll_stat; i++)
        {
            // access the fds of the active sockets
            int fd = events[i].data.fd;

            // know i have the fd but i have to know if it's for server/clietn side
            std::vector<int>::iterator is_server = std::find(server_side_fds.begin(), server_side_fds.end(), fd);
            if (is_server != server_side_fds.end())   // server side
            {
                std::cout << "+ >>> " << fd << " is a SERVER" << std::endl;
            }
            else
            {
                std::cout << "- >>> " << fd << " is a CLIENT" << std::endl;
            }
        }
    }
}
