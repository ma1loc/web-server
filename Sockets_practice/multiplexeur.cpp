# include "sockets.hpp"
# include "set_logs.hpp"
# include <iterator>
# include <algorithm>
# include <sys/stat.h>

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

void socket_engine::remove_fd_from_list(int fd)
{

    std::cout << "--------------------------------------------------------------" << std::endl;
    std::cout << "[>] list befor" << std::endl;
    for (size_t i = 0; i < fds_list.size(); i++)
    {
        std::cout << "fd=" << fds_list.at(i) << " ";
    }
    std::cout << "\n";

    std::vector<int>::iterator fd_position = std::find(fds_list.begin(), fds_list.end(), fd);
    if (fd_position != fds_list.end())
        fds_list.erase(fd_position);


    std::cout << "[>] list after" << std::endl;
    for (size_t i = 0; i < fds_list.size(); i++)
    {
        std::cout << "fd=" << fds_list.at(i) << " ";
    }
    std::cout << "\n";

    std::cout << "--------------------------------------------------------------" << std::endl;
}

void socket_engine::process_connections(void)
{
    // >>> main loop will exist just in one case of signal
    while (true) {
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        epoll_logs(epoll_stat);

        std::cout << "[>] epoll return -> " << epoll_stat << std::endl;

        // ------------------------------------------------------------------- //
        for (int i = 0; i < epoll_stat; i++) {

            // access the fds of the active sockets
            int fd = events[i].data.fd;

            // know i have the fd but i have to know if it's for server/clietn side
            std::vector<int>::iterator is_server = std::find(server_side_fds.begin(), server_side_fds.end(), fd);
            
            if (is_server != server_side_fds.end()) { // Server side 
                std::cout << "[>] Request incoming from Server FD: " << fd << std::endl;

                // accept is like a captuer of the client fds
                int client_fd = accept(fd, NULL, NULL);
                if (client_fd != -1)
                    set_client_side(client_fd);
                else {
                    /*TOKNOW:
                        accept return -1 in case of fd giving have no data yet
                        -1 mean's a lot in case of non-blocking, and is not an error.
                        errno will solve the -1 mean's an error of just the fd not ready yet
                        if errno == EAGAIN -> mean's the client_fd has no data yet in the kernal table
                        if errno == EWOULDBLOCK -> same as the EAGAIN just about the 
                    */
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                        std::cout << "[!] Accept error: "<< strerror(errno) << std::endl;
                }
            }
            else    // Client side
            {
                std::cout << "[>] Data incoming from Client FD: " << fd << std::endl;

                char raw_buffer[BUFFER_SIZE];
                std::memset(raw_buffer, 0, sizeof(raw_buffer));

                int recv_stat = recv(fd, raw_buffer, BUFFER_SIZE, 0);
                std::cout << "[+] Received " << recv_stat << " bytes from fd " << fd << std::endl;


                if (recv_stat > 0) {
                    package_statement[fd].append(raw_buffer, recv_stat);
                    std::cout << "--- DATA START ---\n" << package_statement[fd] << "\n--- DATA END ---" << std::endl;

                    /*  WILL CHECK IF END OF THE REQUSER
                        - if the package_statement[i] fully recv, if yes, will done the response
                        - check the methode of the request
                        - check the end of header request \r\n\r\n
                    */

                    // TOKNOW Content-Length header indicates the size of the message body
                    if (package_statement[fd].find("\r\n\r\n") != std::string::npos)
                    {
                        std::cout << "[>] We get the \\r\\n\\r\\n [<]" << std::endl;
                        if (package_statement[fd].compare(0, 3, "GET") == 0) {
                            std::cout << "[>] We get GET request [<]" << std::endl;

                            // ------------------------- HARDCODED VALUES ------------------------ //
                            // GET /index.html HTTP/1.1
                            std::string method = "GET";
                            std::string path = "/index.html";
                            std::string version = "HTTP/1.1";
                            int status_code = 200;
                            struct stat statbuf;
                            // ------------------------------------------------------------------ //

                            /*  TODO: make the response
                                1# >>> [] check if the path exist '../etc/passwod'
                                2# >>> [] check if accessable too nahhhhhhhhhhhhhh yep fU*
                                3# >>> [] we must use the root: path in the config file
                                4# >>> [] apply status code in case of error, not found
                                5# >>> [] Permission denied (HTTP 403 Forbidden)
                                6# >>> [] check provided root and index in exist and prem
                                7# >>> [] i have to get the index in the "locatoin /"
                                        config file (case of multiple source in index)
                            */
                            if (path == "/")
                                path = "/index.html";
                            std::string full_path = "www" + path;
                            
                            // >>> redirections <<<
                            if (stat(full_path.c_str(), &statbuf) < 0) {
                                std::cout << "[LOG] The requested URL " << path << " was not found on this server." << std::endl;
                                full_path = "www/not-found.html";
                                status_code = 404;
                            }
                            else if (access(full_path.c_str(), F_OK | R_OK) < 0) {
                                std::cout << "[LOG] The requested URL " << path << " is forbidden to access to." << std::endl;
                                full_path = "www/forbidden.html";
                                status_code = 403;
                            }
                            else if (S_ISDIR(statbuf.st_mode)) {
                                full_path += "/index.html";
                                if (access(full_path.c_str(), F_OK | R_OK) < 0) {
                                    std::cout << "[LOG] The requested URL " << path << " is forbidden to access to." << std::endl;
                                    full_path = "www/forbidden.html";
                                    status_code = 403;
                                }
                            }
                            // >>> request handling <<<

                        }
                        else if (package_statement[fd].compare(0, 4, "POST")) {
                            std::cout << "[>] We get POST request [<]" << std::endl;
                            // here will check the content-lenght in the header 
                        }
                        else if (package_statement[fd].compare(0, 6, "DELETE")) {
                            std::cout << "[>] We get DELETE request [<]" << std::endl;
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
