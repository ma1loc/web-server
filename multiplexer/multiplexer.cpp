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
                // std::cerr << "BODY SIZE BEFORE CGI: " << this->raw_client_data[fd].req.getBody().size() << std::endl;
				// std::cerr << "BODY FLAG: " << this->raw_client_data[fd].parse.body << std::endl;
                // std::cout << "[>] after cgiHandler.state: " << this->raw_client_data[fd].cgiHandler.state << std::endl;

				if (this->raw_client_data[fd].cgiHandler.state == CGI_READING)  // it's a CGI
				{
                    // pipe_out_fd -> to read from it
					int pipe_out_fd = this->raw_client_data[fd].cgiHandler.getPipeOutFd();     // pipeOut[0] -> r
                    int pipe_in_fd  = this->raw_client_data[fd].cgiHandler.getPipeInFd();   // pipeIn[1]  -> w

					int ret_out = add_pipeOut_event(fd, pipe_out_fd);
					if (ret_out == -1) {
					close(pipe_out_fd);
					close(pipe_in_fd);
					kill(this->raw_client_data[fd].cgiHandler.pid, SIGTERM);
					this->raw_client_data[fd].cgiHandler.state = ERROR;
					this->raw_client_data[fd].res.set_stat_code(SERVER_ERROR);
					response_builder.build_response();
					}

					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);

					if (this->raw_client_data[fd].parse.body) {
						int ret_in = add_pipeIn_event(fd, pipe_in_fd);
						if (ret_in == -1) {
							close(pipe_in_fd);
							kill(this->raw_client_data[fd].cgiHandler.pid, SIGTERM);
							this->raw_client_data[fd].cgiHandler.state = ERROR;
							this->raw_client_data[fd].res.set_stat_code(SERVER_ERROR);
							response_builder.build_response();
							modify_epoll_event(fd, EPOLLOUT);
							return;
						}
					} else
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
            this->raw_client_data[fd].last_activity = time(0);

            if (raw_client_data[fd].res.stream_response_to_client(fd))
                raw_client_data[fd].close_connection = true;
        }
        else
        {
            std::cout << "exit -> else block" << std::endl;
            
            // First, check if this is a CGI response that needs HTTP status prepended
            if (raw_client_data[fd].cgiHandler.state == CGI_DONE) {
                std::string &cgi_response = raw_client_data[fd].cgiHandler.getResponseRef();
                if (!cgi_response.empty() && cgi_response.find("HTTP/") != 0) {
                    // CGI output doesn't have HTTP status line, prepend it
                    std::string status_line = "HTTP/1.1 200 OK\r\n";
                    cgi_response = status_line + cgi_response;
                    std::cout << "[DEBUG] Prepended HTTP status line" << std::endl;
                }
            }
            
            // Send any buffered CGI output
            std::string &cgi_response = raw_client_data[fd].cgiHandler.getResponseRef();
            std::cout << "[DEBUG] CGI response size: " << cgi_response.size() << " bytes" << std::endl;
            if (!cgi_response.empty())
            {
                std::cout << "[DEBUG] Sending CGI response..." << std::endl;
                ssize_t sent = send(fd, cgi_response.c_str(), cgi_response.size(), MSG_NOSIGNAL);
                std::cout << "[DEBUG] Sent " << sent << " bytes" << std::endl;
                if (sent > 0) {
                    this->raw_client_data[fd].last_activity = time(0);
                    cgi_response.erase(0, sent);
                    if (cgi_response.empty() && raw_client_data[fd].cgiHandler.state == CGI_DONE) {
                        raw_client_data[fd].close_connection = true;
                    }
                }
                else if (sent == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    raw_client_data[fd].close_connection = true;
                }
                return;
            }
            
            // Then send the HTTP response headers (for non-CGI responses)
            std::string buffer = raw_client_data[fd].res.get_raw_response();
            std::cout << "[DEBUG] HTTP response size: " << buffer.size() << " bytes" << std::endl;
            if (!buffer.empty())
            {
                off_t already_sent = raw_client_data[fd].res.get_bytes_sent();
                ssize_t send_stat = send(fd,
                                        buffer.c_str() + already_sent, // ← start from where we left off
                                        buffer.size() - already_sent,  // ← only send what remains
                                        MSG_NOSIGNAL);
                if (send_stat == -1)
                    return;
                if (send_stat > 0) {
                    this->raw_client_data[fd].last_activity = time(0);
                    raw_client_data[fd].res.set_bytes_sent(already_sent + send_stat);
                }
                if (raw_client_data[fd].res.get_bytes_sent() >= (off_t)buffer.size())
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
                client.cgiHandler.reading(fd, events[i].events, client);
                
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
                        rb.set_client(client);
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
                client.cgiHandler.writing(fd, events[i].events, client);
                
                // Check if body is fully written
                if (client.cgiHandler.body_bytes_sent >= (off_t)client.req.getBody().size())
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
