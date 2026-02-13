# include "../socket_engine.hpp"
# include "../response.hpp"
# include "../response_builder.hpp"

# include "../utils/utils.hpp"

void    socket_engine::server_event(ssize_t fd)
{
    std::cout << "[>] Request incoming from Server FD: " << fd << std::endl;    // rm-me

    // accept is like captuer of the client fds
    int client_fd = accept(fd, NULL, NULL);
    if (client_fd != -1)
        init_client_side(client_fd);
    else {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            std::cerr << "[!] accept error: "<< strerror(errno) << std::endl;
    }
}

void    socket_engine::client_event(ssize_t fd)
{
    std::cout << "[>] Data incoming from Client FD: " << fd << std::endl;   // rm-me

    char raw_data[BUFFER_SIZE];
    std::memset(raw_data, 0, sizeof(raw_data));

    int recv_stat = recv(fd, raw_data, BUFFER_SIZE, 0);
    std::cout << "Received " << recv_stat << " bytes from fd " << fd << std::endl;  // rm-me
    
    if (recv_stat > 0) {

        std::cout << "[>] Received " << recv_stat << " bytes from fd " << fd << std::endl;  // rm-me
        std::cout << "[>] --- DATA START ---\n" << raw_data << "\n--- DATA END ---" << std::endl;   // rm-me
        
        // TODO: REQ/RES
        // raw_client_data[fd].req;

        // if (raw_client_data[fd].req_ready)
            std::cout << ">>>>>>>>>>> ENTTTERRRRR <<<<<<<<<<<<" << std::endl;   // rm-me
            response_builder response_builder;
            response_builder.build_response(raw_client_data[fd], server_config_info);
            std::cout << ">>>>>>>>>> STATUS CODE: " << raw_client_data[fd].res.get_stat_code() << " <<<<<<<<<<" << std::endl;   // rm-me
    } else {
        close (fd);
        raw_client_data.erase(fd);
        remove_fd_from_list(fd);
        if (recv_stat == 0)  // EOF
            std::cerr << "[!] Client lost connection" << std::endl;
        else
            std::cerr << "[!] Client connection broke: " << strerror(errno) << std::endl;
    }
}


void    socket_engine::process_connections(void)
{
    while (true)
    {
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        for (int i = 0; i < epoll_stat; i++)
        {
            std::string xxx = stat_code_to_string(200);
            // access the fds of the active sockets
            int fd = events[i].data.fd;

            // int fd is a server fd or clietn fd?
            std::vector<int>::iterator is_server = std::find(server_side_fds.begin(), server_side_fds.end(), fd);
            if (is_server != server_side_fds.end()) // server event -> done[*]
                server_event(fd);
            else    // client event -> done[]
                client_event(fd);
        }
    }
}
