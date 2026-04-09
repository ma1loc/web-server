# include "../socket_engine.hpp"
# include "../response.hpp"
# include "../response_builder.hpp"
# include "../utils/utils.hpp"
# include "../client.hpp"
# include "../request/includes/request.hpp"
# include "../request/includes/parseRequest.hpp"

void socket_engine::server_event(ssize_t fd)    // DONE [+]
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        std::cerr << READ_S << "[!] Accept failed on FD: " << fd << ": "
            << std::strerror(errno) << READ_E << std::endl;
        return;
    }
    char    *client_ip = inet_ntoa(client_addr.sin_addr);
    int     client_port = ntohs(client_addr.sin_port);
    std::cout << GREEN_S << "[+] New Connection from " << client_ip 
            << ":" << client_port << " on FD " << client_fd 
            << GREEN_E << std::endl;

    init_client_side(client_fd);
}

void    socket_engine::client_event(ssize_t fd, uint32_t events) // DONE []
{
    if (events & (EPOLLHUP | EPOLLERR)) {
        terminate_client(fd, "[-] RST: network connection dropped/Error ");
        return ;
    }
    if (events & EPOLLRDHUP) {    // 'FIN'
        if (!(events & EPOLLIN)) {    // 'FIN' & 'no data'
            terminate_client(fd, "[-] Client closed connection");
            return ;
        }
    }

    if (events & EPOLLIN)   // READY TO READ
    {
        char raw_data[BUFFER_SIZE];
        std::memset(raw_data, 0, sizeof(raw_data));

        int recv_stat = recv(fd, raw_data, BUFFER_SIZE, 0);
        if (recv_stat > 0)
        {
            this->raw_client_data[fd].last_activity = time(0);
            std::string raw_data_buff(raw_data, recv_stat);

            // rm-me
            std::cout << READ_S << "--------- START REQUEST\n" << raw_data_buff << "\n------- END RAQUEST" << READ_E << std::endl;

            int req_stat = parseRequest(this->raw_client_data[fd], raw_data_buff);
            if (req_stat == REQ_NOT_READY)
                return ;
            


            // ------------------------------- CGI -----------------------------------
            // CGI_NOT_REQUIRED, CGI_DONE, ERROR
            this->raw_client_data[fd].cgiHandler.handleCGI(this->raw_client_data[fd]);
            // -----------------------------------------------------------------------
            if (this->raw_client_data[fd].cgiHandler.state != CGI_NOT_REQUIRED)
            {
                std::cout << "IT'S FUCKING CGI" << std::endl;
                return ;
            }
            else {
                std::cout << "IT'S NOT FUCKING CGI" << std::endl;
            }
            // -----------------------------------------------------------------------

            this->raw_client_data[fd].res.set_stat_code(req_stat);

            // -------------------------------------------------------------------------------
            response_builder response_builder;
            response_builder.init_response_builder(raw_client_data[fd]);

            response_builder.build_response();
            
            modify_epoll_event(fd, EPOLLOUT | EPOLLIN);
            // -------------------------------------------------------------------------------

        }
        else if (recv_stat == 0)
            terminate_client(fd, "[!] Client lost connection (EOF)");
        else
            terminate_client(fd, "[!] Client connection broke");
    }

    if (events & EPOLLOUT)
    {
        if (raw_client_data[fd].is_serving_file)
        {
            // rm-me
            std::cout << GREEN_S << "+++++++++++++++++++++++++++++++++ YOUR SERVING STATIC FILE +++++++++++++++++++++++++++++++++" << GREEN_E << std::endl;

            if (raw_client_data[fd].res.stream_response_to_client(fd))
                raw_client_data[fd].close_connection = true;
        }
        else {
            // exit(1);
            std::string buffer = raw_client_data[fd].res.get_raw_response();
            if (!buffer.empty())
            {
                ssize_t send_stat = send(fd, buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
                if (send_stat == -1)
                    return ;
                if ((ssize_t)buffer.size() == send_stat)
                    raw_client_data[fd].close_connection = true;
            }
            if (buffer.empty())
                    raw_client_data[fd].close_connection = true;
        }
    }
}


void socket_engine::process_connections(void)   // main func about events
{
    while (true)
    {
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        for (int i = 0; i < epoll_stat; i++)
        {
            int fd = events[i].data.fd;

            if (is_server(server_side_fds, fd))
                server_event(fd);   // new-client
            
            // -> modify_epoll_event(client_fd, EPOLLOUT); -> to read the result of the CGI
            else if (pipe_to_client.count(fd))   // read CGI result
            {
                // here will read from CGI pipe and stream to client
                int client_fd = pipe_to_client[fd];
                Client &client = raw_client_data[client_fd];
                client.last_activity = time(0);
                std::cout << "[DEBUG] process_connections: handling pipe_to_client fd=" << fd << " client_fd=" << client_fd << std::endl;
                // client.cgiHandler.reading(fd, events[i].events, client);
                
                // Check if CGI is done
                if (client.cgiHandler.state == CGI_DONE || client.cgiHandler.state == ERROR)
                {
                    std::cout << "[DEBUG] CGI done, closing pipe " << fd << " and re-adding client " << client_fd << std::endl;
                    int del_ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    if (del_ret == -1)
                        std::cerr << "[!] epoll_ctl DEL failed: " << strerror(errno) << std::endl;
                    close(fd);
                    pipe_to_client.erase(fd);
                    remove_fd_from_list(fd);
                    
                    // Build HTTP response with status and headers
                    if (client.cgiHandler.state == ERROR) {
                        client.res.set_stat_code(SERVER_ERROR);
                        response_builder rb;
                        // rb.set_client(client);
                        rb.build_response();
                    } else {
                        // Build response with CGI output as body
                        client.res.set_stat_code(OK);
                        std::cout << "[DEBUG] Building response for CGI output..." << std::endl;
                        // TODO: Parse CGI headers if needed, for now just send output as-is
                        // The CGI handler captures the full HTTP response from the script
                    }
                    
                    // Re-add client socket for response send
                    modify_epoll_event(client_fd, EPOLLOUT);
                }
            }
            // will be write just in case has a body
            // request to send it to CGI
            else if (pipe_write_to_client.count(fd))
            {
                int client_fd = pipe_write_to_client[fd];
                Client &client = raw_client_data[client_fd];
                client.last_activity = time(0);
                // client.cgiHandler.writing(fd, events[i].events, client);
                
                // Check if body is fully written
                // if (client.cgiHandler.body_bytes_sent >= (off_t)client.req.getBody().size())
                {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    pipe_write_to_client.erase(fd);
                    remove_fd_from_list(fd);
                }
            }

            else    // serv client event - CGI check
                client_event(fd, events[i].events);
        }
        check_all_client_timeouts();
    }
}
