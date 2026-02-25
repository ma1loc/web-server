 # include "../socket_engine.hpp"
# include "../response.hpp"
# include "../response_builder.hpp"
# include "../utils/utils.hpp"

// -------------------- HARDCODED --------------------------
static std::string _host_ = "10.11.11.4";
static int _port_ = 8080;

// static std::string _host_ = "localhost";
// static int _port_ = 9090;
// -------------------------------------------------------
void socket_engine::server_event(ssize_t fd)
{
    int client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0) {
        std::cerr << "[!] Accept failed on FD: " << fd << ": " << std::strerror(errno) << std::endl;
        return;
    }
    init_client_side(client_fd);
}

/*  TODO:
    create EPOLLIN ready to read methode
    create EPOLLOUT ready to write methode
*/
void    socket_engine::client_event(ssize_t fd, uint32_t events)
{
    std::cout << "[>] Data incoming from Client FD: " << fd << std::endl;   // rm-me
    
    if (events & EPOLLIN) {
        char raw_data[BUFFER_SIZE];
        std::memset(raw_data, 0, sizeof(raw_data));

        int recv_stat = recv(fd, raw_data, BUFFER_SIZE, 0);
        std::cout << "Received " << recv_stat << " bytes from fd " << fd << std::endl;  // rm-me
        if (recv_stat > 0) {
            this->raw_client_data[fd].last_activity = time(0);

            // --------------- temp inter request ready --------------
            this->raw_client_data[fd].host = _host_;
            this->raw_client_data[fd].port = _port_;
            // -------------------------------------------------------

            std::cout << "[>] Received " << recv_stat << " bytes from fd " << fd << std::endl;  // rm-me
            std::cout << "[>] --- DATA START ---\n" << raw_data << "\n--- DATA END ---" << std::endl;   // rm-me

            // NOTE: REQUEST MOST START FIRST HERE //

            // if (raw_client_data[fd].req_ready)
                std::cout << ">>>>>>>>>>> ENTTTERRRRR <<<<<<<<<<<<" << std::endl;   // rm-me
                
                // NOTE: it's only job is make the response ready
                response_builder response_builder;  // it's just a temprary class 
                // build_response it's job to init the client &current_client to use it later
                response_builder.build_response(raw_client_data[fd], server_config_info);
            
            // NOTE: CALLING modify_epoll_event MEAN'S response full ready //
            modify_epoll_event(fd, EPOLLOUT | EPOLLIN);
        } else {
            if (recv_stat == 0)  // EOF
                terminate_client(fd, "[!] Client lost connection");
            else
                terminate_client(fd, "[!] Client connection broke");
        }
    }
    if (events & EPOLLOUT)
    {   
        std::string buffer_knowon = raw_client_data[fd].res.get_raw_response();
        if (!buffer_knowon.empty()) {

            ssize_t send_stat = send(fd, buffer_knowon.c_str(), buffer_knowon.size(), 0);
            if (send_stat > 0)
                buffer_knowon.erase(buffer_knowon.begin(), buffer_knowon.begin() + send_stat);
            else if (send_stat == -1)
                return ;
        }

        if (buffer_knowon.empty())
            terminate_client(fd, "Response sent Successfully (HTTP/1.0)");
    }
}

void    socket_engine::process_connections(void)
{
    while (true)
    {
        // >>> epoll_fd -> is the pipe of the fds_table in the kernal
        // >>> events -> is a array that hold the active fds
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        for (int i = 0; i < epoll_stat; i++)
        {
            // >>> Get access to ready
            int fd = events[i].data.fd;

            // ------------------------------------------------------------------------------------------------
            // int fd is a server fd or clietn fd?
            std::vector<int>::iterator is_server = std::find(server_side_fds.begin(), server_side_fds.end(), fd);
            if (is_server != server_side_fds.end()) // server event -> done[*]
                server_event(fd);   // Accept + Set time(0) [*]
            else    // client event -> done[]
                client_event(fd, events[i].events);   // Recv/Send + Update time(0) [*]
            // ------------------------------------------------------------------------------------------------
        }
        // in this function i have to get the server config file timeout and the now - last_activity
        check_all_client_timeouts();
    }
}
