# include "../socket_engine.hpp"

void socket_engine::process_connections(void)
{
    while (true) {
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        std::cout << "[>] epoll return -> " << epoll_stat << std::endl;

        for (int i = 0; i < epoll_stat; i++) {

            // access the fds of the active sockets
            int fd = events[i].data.fd;

            // int fd is a server fd or clietn fd?
            std::vector<int>::iterator is_server = std::find(server_side_fds.begin(), server_side_fds.end(), fd);
            
            if (is_server != server_side_fds.end()) {   // server event -> done[*]
                std::cout << "[>] Request incoming from Server FD: " << fd << std::endl;

                // accept is like captuer of the client fds
                int client_fd = accept(fd, NULL, NULL);
                if (client_fd != -1)
                    init_client_side(client_fd);
                else {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                        std::cout << "[!] Accept error: "<< strerror(errno) << std::endl;
                }
            }
            else    // client event -> done[]
            {
                std::cout << "[>] Data incoming from Client FD: " << fd << std::endl;

                char raw_buffer[BUFFER_SIZE];
                std::memset(raw_buffer, 0, sizeof(raw_buffer));

                int recv_stat = recv(fd, raw_buffer, BUFFER_SIZE, 0);
                std::cout << "Received " << recv_stat << " bytes from fd " << fd << std::endl;
                
                if (recv_stat > 0) {

                    std::cout << "[>] Received " << recv_stat << " bytes from fd " << fd << std::endl;
                    std::cout << "[>] --- DATA START ---\n" << raw_buffer << "\n--- DATA END ---" << std::endl;
                    
                    // TODO: REQUEST PARSING
                    // NOTE access will be like this:
                        // raw_client_data[fd].req....
                        // TODOKNOW: parssing will start when you get 100% of the header by checking \r\n\r\n
                    // what i need:
                    /*
                        method
                        path
                        protocol
                        host -> localhost:8080 -> (virtual host) will skip that
                        content-lenght -> body size in byte
                        NOTE in the request, if there's no key:value style ignore it, else save, just in case of the header
                    */
                    void    request_handler(std::string raw_buffer); // example

                    // ------------------------------------------------------------------------------------- //
                    // if (raw_client_data[fd].req_ready && raw_client_data[fd].req.get_request_stat() == 0) {
                        // here i will build the response based on your request
                    // }
                    
                }
                else if (recv_stat == 0) {  // client send nothing and close the connection
                    close (fd);
                    raw_client_data.erase(fd);
                    remove_fd_from_list(fd);
                    std::cout << "[!] Client lost connection: " << strerror(errno) << std::endl;
                }
                else
                    std::cerr << "[-] Error recv failed: " << strerror(errno) << std::endl;

            }
        }
    }
}
