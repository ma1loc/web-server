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
    if (events & (EPOLLHUP | EPOLLERR)) {
        terminate_client(fd, "[-] RST: network connection dropped/Error ");
        return;
    }
    if (events & EPOLLRDHUP) { // 'FIN'
        if (!(events & EPOLLIN)) { // 'FIN' & 'no data'
            terminate_client(fd, "[-] Client closed connection");
            return;
        }
    }

    if (events & EPOLLIN) // READY TO READ
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

            // TODO: fixed method not allowed
            std::cout << "req_stat -> " << req_stat << std::endl;

            response_builder response_builder;
            if (response_builder.init_response_builder(raw_client_data[fd]) && req_stat == OK)
            {
                std::cout << "+++++++++++++++++++++++ ENTER CGI +++++++++++++++++++++++" << std::endl;

                // -----------------------------------------------------------------------
                // CGI_NOT_REQUIRED, CGI_DONE, ERROR
                this->raw_client_data[fd].cgiHandler.handleCGI(this->raw_client_data[fd]);
                std::cerr << "BODY SIZE BEFORE CGI: " << this->raw_client_data[fd].req.getBody().size() << std::endl;
				std::cerr << "BODY FLAG: " << this->raw_client_data[fd].parse.body << std::endl;


				if (this->raw_client_data[fd].cgiHandler.state == CGI_READING)
				{
					int pipe_out_fd = this->raw_client_data[fd].cgiHandler.getPipeFd();
					int pipe_in_fd  = this->raw_client_data[fd].cgiHandler.getPipeInFd();

					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);

					struct epoll_event ev;
					ev.events = EPOLLIN;
					ev.data.fd = pipe_out_fd;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_out_fd, &ev);
					this->pipe_to_client[pipe_out_fd] = fd;
					set_fds_list(pipe_out_fd);

					if (this->raw_client_data[fd].parse.body)
					{
						ev.events = EPOLLOUT;
						ev.data.fd = pipe_in_fd;
						epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_in_fd, &ev);
						this->pipe_write_to_client[pipe_in_fd] = fd;
						set_fds_list(pipe_in_fd);
					}
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

    // if (events & EPOLLOUT)
    // {
    //     if (raw_client_data[fd].is_serving_file)
    //     {
    //         // rm-me
    //         std::cout << GREEN_S << "+++++++++++++++++++++++++++++++++ YOUR SERVING STATIC FILE +++++++++++++++++++++++++++++++++" << GREEN_E << std::endl;

    //         if (raw_client_data[fd].res.stream_response_to_client(fd))
    //             raw_client_data[fd].close_connection = true;
    //     }
    //     else {
    //         // exit(1);
    //         std::string buffer = raw_client_data[fd].res.get_raw_response();
    //         if (!buffer.empty())
    //         {
    //             ssize_t send_stat = send(fd, buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
    //             if (send_stat == -1)
    //                 return ;
    //             if ((ssize_t)buffer.size() == send_stat)
    //                 raw_client_data[fd].close_connection = true;
    //         }
    //         if (buffer.empty())
    //                 raw_client_data[fd].close_connection = true;
    //     }
    // }
    if (events & EPOLLOUT)
    {
        if (raw_client_data[fd].is_serving_file)
        {
            std::cout << BLUE << "is_serving_fileeeeeeeeeeeeeeeeeeeeeeeeee" << RESET << std::endl;

            if (raw_client_data[fd].res.stream_response_to_client(fd))
                raw_client_data[fd].close_connection = true;
        }
        else
        {
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

void socket_engine::process_connections(void)
{
    while (true)
    {
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        for (int i = 0; i < epoll_stat; i++)
        {
            int fd = events[i].data.fd;

            // >>> fd is a server fd or clietn fd
            std::vector<int>::iterator is_server = std::find(server_side_fds.begin(), server_side_fds.end(), fd);

            if (is_server != server_side_fds.end())
                server_event(fd);
            // else if (pipe_to_client.count(fd))
            // {
            //     std::cout << BLUE << "++++++++++++++++++++++++ It's CGI ++++++++++++++++++++++++" << RESET << std::endl;

            //     int client_socket = pipe_to_client[fd];
				
            //     this->raw_client_data[client_socket].cgiHandler.reading();

            //     cgiState stat = this->raw_client_data[client_socket].cgiHandler.state;
			// 	if (stat == CGI_DONE || stat == ERROR)
			else if (pipe_to_client.count(fd))
			{
				int client_socket = pipe_to_client[fd];
				this->raw_client_data[client_socket].last_activity = time(0); 
				this->raw_client_data[client_socket].cgiHandler.reading();
				
				cgiState stat = this->raw_client_data[client_socket].cgiHandler.state;
				if (stat == CGI_DONE || stat == ERROR) {
					// ... (your existing cleanup and response building logic)
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
					remove_fd_from_list(fd);
					pipe_to_client.erase(fd);

					raw_client_data[client_socket].last_activity = time(0);

					if (stat == CGI_DONE) {
						response_builder rb;
						rb.set_client(raw_client_data[client_socket]);
						rb.build_cgi_response(raw_client_data[client_socket].cgiHandler.getResponse());
					} else {
						raw_client_data[client_socket].res.set_stat_code(SERVER_ERROR);
						response_builder rb;
						rb.set_client(raw_client_data[client_socket]);
						rb.build_response();
					}

					// ← Use ADD instead of MOD since client was removed from epoll
					struct epoll_event ev;
					ev.events = EPOLLOUT;
					ev.data.fd = client_socket;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &ev);
				}
            }
			else if (pipe_write_to_client.count(fd))
			{

				int client_socket = pipe_write_to_client[fd];
				Client &c = raw_client_data[client_socket];
				const std::string &body = c.req.getBody();
				off_t &sent = c.cgiHandler.body_bytes_sent;
				
				// rm-me
				std::cerr << "PIPE_WRITE FIRED sent=" << c.cgiHandler.body_bytes_sent << "/" << body.size() << std::endl;
				
				ssize_t n = write(fd, body.c_str() + sent, body.size() - sent);
				std::cerr << "write returned: " << n << " errno: " << errno << " remaining: " << (body.size() - sent) << std::endl;
				if (n > 0) {
					sent += n;
					raw_client_data[client_socket].last_activity = time(0);
				}

				if (sent >= (off_t)body.size())
				{
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
					remove_fd_from_list(fd);
					pipe_write_to_client.erase(fd);
					close(fd);
				}
			}
            else
                client_event(fd, events[i].events);
        }
        check_all_client_timeouts();
    }
}
