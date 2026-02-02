# include "../socket_engine.hpp"

void socket_engine::process_connections(void)
{
    while (true) {
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        std::cout << "[>] epoll return -> " << epoll_stat << std::endl;
        // epoll_logs(epoll_stat);
        // ------------------------------------------------------------------- //

        for (int i = 0; i < epoll_stat; i++) {

            // access the fds of the active sockets
            int fd = events[i].data.fd;

            // int fd is a server fd or clietn fd?
            std::vector<int>::iterator is_server = std::find(server_side_fds.begin(), server_side_fds.end(), fd);
            
            if (is_server != server_side_fds.end()) {   // server event
                std::cout << "[>] Request incoming from Server FD: " << fd << std::endl;

                // accept is like a captuer of the client fds
                int client_fd = accept(fd, NULL, NULL);
                if (client_fd != -1)
                    init_client_side(client_fd);
                else {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                        std::cout << "[!] Accept error: "<< strerror(errno) << std::endl;
                }
            }
            else    // client event
            {
                std::cout << "[>] Data incoming from Client FD: " << fd << std::endl;

                char raw_buffer[BUFFER_SIZE];
                std::memset(raw_buffer, 0, sizeof(raw_buffer));

                int recv_stat = recv(fd, raw_buffer, BUFFER_SIZE, 0);
                std::cout << "Received " << recv_stat << " bytes from fd " << fd << std::endl;
                
                if (recv_stat > 0) {
                    std::cout << "[>] Received " << recv_stat << " bytes from fd " << fd << std::endl;
                    package_statement[fd].append(raw_buffer, recv_stat);
                    std::cout << "[>] --- DATA START ---\n" << package_statement[fd] << "\n--- DATA END ---" << std::endl;

                    // TOKNOW Content-Length header indicates the size of the message body
                    if (package_statement[fd].find("\r\n\r\n") != std::string::npos)
                    {
                        std::cout << "[>] We get the \\r\\n\\r\\n [<]" << std::endl;
                        if (package_statement[fd].compare(0, 3, "GET") == 0) {  // just a test
                            std::cout << "[>] GET request [<]" << std::endl;

                        }
                        else if (package_statement[fd].compare(0, 4, "POST")) {
                            std::cout << "[>] POST request [<]" << std::endl;
                            // here will check the content-lenght in the header 
                        }
                        else if (package_statement[fd].compare(0, 6, "DELETE")) {
                            std::cout << "[>] DELETE request [<]" << std::endl;
                        }
                    }

                    // TOKNOW: parsing of the request will needed here ()
                }


                else if (recv_stat == 0) {
                    // here i have to remove the clietn form the fd list and close it's fd 
                    close (fd);
                    package_statement.erase(fd);
                    remove_fd_from_list(fd);
                    std::cout << "[!] Client lost connection: " << strerror(errno) << std::endl;
                }
                else
                    std::cerr << "[-] recv error on fd " << events[i].data.fd << ": " << strerror(errno) << std::endl;

            }
        }
    }
}
