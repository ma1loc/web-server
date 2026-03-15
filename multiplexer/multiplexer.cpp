#include "../socket_engine.hpp"
#include "../response.hpp"
#include "../response_builder.hpp"
#include "../utils/utils.hpp"
#include "../client.hpp"
#include "../request/includes/request.hpp"
#include "../request/includes/parseRequest.hpp"

void socket_engine::server_event(ssize_t fd) // DONE [+]
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0)
    {
        std::cerr << READ_S << "[!] Accept failed on FD: " << fd << ": "
                  << std::strerror(errno) << READ_E << std::endl;
        return;
    }
    char *client_ip = inet_ntoa(client_addr.sin_addr);
    int client_port = ntohs(client_addr.sin_port);
    std::cout << GREEN_S << "[+] New Connection from " << client_ip
              << ":" << client_port << " on FD " << client_fd
              << GREEN_E << std::endl;

    init_client_side(client_fd);
}

void socket_engine::client_event(ssize_t fd, uint32_t events)
{
    if (events & (EPOLLHUP | EPOLLERR))
    {
        terminate_client(fd, "[-] RST: network connection dropped/Error ");
        return;
    }
    if (events & EPOLLRDHUP)
    {   // 'FIN'
        if (!(events & EPOLLIN)) { // 'FIN' & 'no data'
            terminate_client(fd, "[-] Client closed connection");
            return;
        }
    }

    if (events & EPOLLIN) // READY TO READ | TODO: CGI FIXED
    {
        std::cout << BLUE << "EPOLLINNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN" << RESET << std::endl;
        char raw_data[BUFFER_SIZE];
        std::memset(raw_data, 0, sizeof(raw_data));

        int recv_stat = recv(fd, raw_data, BUFFER_SIZE, 0);
        if (recv_stat > 0)
        {
            this->raw_client_data[fd].last_activity = time(0);
            std::string raw_data_buff(raw_data, recv_stat);

            // rm-me
            std::cout << READ_S << "--------- START REQUEST\n"
                      << raw_data_buff << "\n------- END RAQUEST" << READ_E << std::endl;

            int req_stat = parseRequest(this->raw_client_data[fd], raw_data_buff);
            if (req_stat == REQ_NOT_READY)
                return;
            this->raw_client_data[fd].res.set_stat_code(req_stat);

            // rm-me
            std::cout << "req_stat -> " << req_stat << std::endl;

            response_builder response_builder;
            if (response_builder.init_response_builder(raw_client_data[fd]) && req_stat == OK)
            {
                // -----------------------------------------------------------------------
                // CGI_NOT_REQUIRED, CGI_DONE, ERROR
                
                std::cout << "-> before cgiHandler.state" << this->raw_client_data[fd].cgiHandler.state << std::endl;
                
                this->raw_client_data[fd].cgiHandler.handleCGI(this->raw_client_data[fd]);
                
                // rm-me
                std::cerr << "BODY SIZE BEFORE CGI: " << this->raw_client_data[fd].req.getBody().size() << std::endl;
				std::cerr << "BODY FLAG: " << this->raw_client_data[fd].parse.body << std::endl;
                std::cout << "[>] after cgiHandler.state: " << this->raw_client_data[fd].cgiHandler.state << std::endl;

				if (this->raw_client_data[fd].cgiHandler.state == CGI_READING)  // it's a CGI
				{
                    // pipe_out_fd -> to read from it
					int pipe_out_fd = this->raw_client_data[fd].cgiHandler.getPipeOutFd();     // pipeOut[0] -> r
                    int pipe_in_fd  = this->raw_client_data[fd].cgiHandler.getPipeInFd();   // pipeIn[1]  -> w

					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);

					add_pipeOut_event(fd, pipe_out_fd);  // pipeOut[0] -> EPOLLIN  (read CGI output)
					if (this->raw_client_data[fd].parse.body)
						add_pipeIn_event(fd, pipe_in_fd);   // pipeIn[1]  -> EPOLLOUT (write body to CGI)
					else
						close(pipe_in_fd);
					return;
				}
                if (this->raw_client_data[fd].cgiHandler.state == ERROR)
                    this->raw_client_data[fd].res.set_stat_code(SERVER_ERROR);
                else
                    this->raw_client_data[fd].res.set_stat_code(req_stat);
            }
            // -------------------------------------------------------------------------------
            
            std::cout << "this->raw_client_data[fd].res.get_stat_code() -> " << this->raw_client_data[fd].res.get_stat_code() << std::endl;
            
			std::cout << "response_builder.build_response() CALLLED" << std::endl;
			// sleep(2);
            response_builder.build_response();
            modify_epoll_event(fd, EPOLLOUT);
            // modify_epoll_event(fd, EPOLLOUT | EPOLLIN);
            // -------------------------------------------------------------------------------
        }
        else if (recv_stat == 0)
            terminate_client(fd, "[!] Client lost connection (EOF)");
        else
            terminate_client(fd, "[!] Client connection broke");
    }

    if (events & EPOLLOUT)  // READY TO WRITE
    {
        if (raw_client_data[fd].is_serving_file)    // serving static file using file fd
        {
            std::cout << "exit -> is_serving_file" << std::endl;
            std::cout << BLUE << "SERVING STATIC FILE" << RESET << std::endl;

            if (raw_client_data[fd].res.stream_response_to_client(fd))
                raw_client_data[fd].close_connection = true;
        }
        else
        {
            std::cout << "exit -> else block" << std::endl;
            // exit(1);

            std::string buffer = raw_client_data[fd].res.get_raw_response();
            if (!buffer.empty())
            {
                off_t already_sent = raw_client_data[fd].res.get_bytes_sent();
                ssize_t send_stat = send(fd,
                                        buffer.c_str() + already_sent, // ← start from where we left off
                                        buffer.size() - already_sent,  // ← only send what remains
                                        MSG_NOSIGNAL);
                if (send_stat == -1)
                    return;
                if (send_stat > 0)
                    raw_client_data[fd].res.set_bytes_sent(already_sent + send_stat);
                if (raw_client_data[fd].res.get_bytes_sent() >= (off_t)buffer.size())
                    raw_client_data[fd].close_connection = true;
            }
            if (buffer.empty())
                raw_client_data[fd].close_connection = true;
        }
    }
}

void socket_engine::process_connections(void) // main func about events
{
    while (true)
    {
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        for (int i = 0; i < epoll_stat; i++)
        {
            int fd = events[i].data.fd;
            if (is_server(server_side_fds, fd))
                server_event(fd);// new-client
            // if (pipe_to_client.count(fd))
            //     cgi_reading(fd, events);
            // else if (pipe_write_to_client.count(fd))
            //     cgi_writing(fd, events);
            else
                client_event(fd, events[i].events); // serv client event 
        }
        check_all_client_timeouts();
    }
}
