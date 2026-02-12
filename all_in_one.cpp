// mutiiplixer header file
# ifndef SOCKET_ENGINE_HPP
# define SOCKET_ENGINE_HPP

# include <sys/epoll.h>
# include <string>
# include <vector>
# include <map>

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <cstring>
# include <stdint.h>
# include <cerrno>
# include <cstdlib>
# include <fcntl.h>
# include <netdb.h>
# include <iostream>
# include <algorithm>
# include <deque>

# include "request.hpp"
# include "response.hpp"
# include "config_parsing/ConfigPars.hpp"

# define TIMEOUT 1000
# define QUEUE_LIMIT 128
# define BUFFER_SIZE 1024
# define PROTOCOL_TYLE 0
# define MAX_EVENTS 64

struct client
{
    request req;
    response res;
    std::string remaining;
    bool req_ready;
};

class socket_engine {
    private:
        int epoll_fd;   // ID for the table
        struct epoll_event events[MAX_EVENTS];
        std::vector<int> server_side_fds;   // >>> backup for the server socket fds
        std::vector<int> fds_list;  // >>> backup for all the fds used to free them in case of SIGINT
        
        std::map<int, client> raw_client_data; // >>> raw request data stored in

        std::deque<ServerBlock> server_config_info; // >>> config file saved here

    public:
        socket_engine();
        void    init_client_side(int fd);
        void    init_server_side(std::string port, std::string host);

        void    process_connections(void);  // here i have to mutiplixier loop
        void    remove_fd_from_list(int fd);
        void    free_fds_list(void);

        void    set_fds_list(int fd);
        void    set_server_side_fds(int s_fd);
        void    set_server_config_info(std::deque<ServerBlock> server_config);
        
        std::vector<int>        get_server_side_fds(void);
        std::map<int, client>   &get_raw_client_data(void);
        std::deque<ServerBlock> get_server_config_info(void);
};

# endif

// response header file:
# ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <map>
# include <string>
# include <vector>
# include "request.hpp"
# include "config_parsing/ConfigPars.hpp"

# define OK 200
# define FORBIDDEN_ACCESS 403
# define NOT_FOUND 404
# define METHOD_NOT_ALLOWED 405
# define SERVER_ERROR 500

class response  // DONE[]
{
    private:
        unsigned short int  stat_code;
        ssize_t             content_length;
        std::string         path;
        std::map<std::string, std::string> header;
        bool                is_body_ready;
        std::string         body;

    public:
        response();

        // SETTERS
        void    set_stat_code(unsigned short int stat_code);
        void    set_body_contnet(std::string body);
        void    set_path(std::string &re_path);
        void    set_body_as_ready(void);
        
        // GETTERS
        std::string         get_str_stat_code(unsigned short int code);
        unsigned short int  get_stat_code(void);
        ssize_t             get_content_length(void);
        std::string         get_path(void);
        bool                get_is_body_ready(void);
};

# endif

// respoppnse cpp file:
response::response() {
    this->stat_code = 200;
    this->content_length = -1;
    this->is_body_ready = false;
    this->path = "";
};

void    response::set_stat_code(unsigned short int stat_code) {
    this->stat_code = stat_code;
}

void    response::set_body_contnet(std::string body) {
    this->body = body;
    content_length = body.length();
}

void    response::set_body_as_ready(void) {
    this->is_body_ready = true;
}

void    response::set_path(std::string &re_path) {
    this->path = re_path;
}

unsigned short int response::get_stat_code(void) {
    return (this->stat_code);
}

ssize_t   response::get_content_length(void) {
    return (this->content_length);
}

std::string response::get_path(void) {
    return (this->path);
}

bool    response::get_is_body_ready(void) {
    return (this->is_body_ready);
}

// response_builder header file:
# ifndef RESPONSE_BUILDER_HPP
# define RESPONSE_BUILDER_HPP

# include <deque>
# include <vector>
# include "response.hpp"
# include "./config_parsing/ConfigPars.hpp"

struct client;

class response_builder
{
    private:
        bool        is_allowd_method(std::deque<std::string> allow_methods, std::string method);
        void        path_validation(const LocationBlock *location_block, response &res);
        std::string path_resolver(std::string root, std::string path);
        std::string index_file_iterator(std::string &full_path, const std::deque<std::string> &index);
        std::string html_index_gen(std::vector<std::string> &dir_list, std::string &uri_path);
        void        autoindex_page(std::string &full_path, response &res);
    public:
        response_builder();
        void build_response(client &current_client, std::deque<ServerBlock> &config);
};

# endif

// response_builder cpp file:
# include "../response_builder.hpp"
# include "../socket_engine.hpp"    // used just to indelucde the client struct //
# include <sys/stat.h>
# include <dirent.h>


// void build_response(client &current_client, std::deque<ServerBlock> &config);
// response_builder::response_builder(std::deque<ServerBlock> &server_config_info)

// REQUEST HARDCODED VALUES TO TEST
std::string method = "GET";
std::string protocol = "HTTP/1.0";
std::string path = "/";
std::string host = "localhost";
int port = 8080;

response_builder::response_builder() {};

bool    response_builder::is_allowd_method(std::deque<std::string> allow_methods, std::string method)
{
    for (size_t i = 0; i < allow_methods.size(); i++) {
        if (allow_methods.at(i) == method)
            return (true);
    }
    return (false);
}

std::string response_builder::index_file_iterator(std::string &full_path ,const std::deque<std::string> &index)
{
    std::string redirection_path;
    std::string based_path = full_path;
    if (!full_path.empty() && full_path.at(full_path.length() -1) != '/')
        based_path += '/';

    for (size_t i = 0; i < index.size(); i++)
    {
        redirection_path = based_path + index.at(i);
        if (access(redirection_path.c_str(), F_OK | R_OK) == 0)
            return (redirection_path);
    }
    return ("");
}

void    response_builder::autoindex_page(std::string &full_path, response &res)
{
    std::cout << "////////// autoindex_page call //////////" << std::endl;

    DIR *dir = opendir(full_path.c_str());
    if (dir == NULL)
        return (res.set_stat_code(FORBIDDEN_ACCESS), (void)0);
    std::vector<std::string> dir_list;
    while (true)
    {
        dirent *read_dir = readdir(dir);
        if (read_dir == NULL)
            break;
        dir_list.push_back(read_dir->d_name);
    }
    std::string html_gen = html_index_gen(dir_list, full_path);
    res.set_body_contnet(html_gen);
    std::cout << html_gen << std::endl;
    res.set_body_as_ready();
    closedir(dir);
}

std::string response_builder::html_index_gen(std::vector<std::string> &dir_list, std::string &uri_path)
{
    std::string gen_html = "<html>\n<body>\n<h1>Index of " + uri_path + "</h1>\n<hr>\n";

    gen_html.append("<a href=\"../\">../</a><br>\n");

    for (size_t i = 0; i < dir_list.size(); i++) {
        std::string &name = dir_list.at(i);

        if (name == ".") continue;
        if (name == "..") continue;

        gen_html.append("<a href=\"" + name + "\">" + name + "</a><br>\n");
    }
    gen_html.append("<hr>\n</body>\n</html>");
    return (gen_html);
}

std::string response_builder::path_resolver(std::string root, std::string path)
{
    if (!root.empty() && root.at(root.length() - 1) == '/')
        root.erase(root.length() - 1);
    return (root + path);
}

void    response_builder::path_validation(const LocationBlock *location_block, response &res)
{
    struct stat statbuf;

    std::string full_path = path_resolver(location_block->root, res.get_path());
    if (stat(full_path.c_str(), &statbuf) < 0 || access(full_path.c_str(), R_OK) < 0)
        return (res.set_stat_code(NOT_FOUND), (void)0);
    if (S_ISDIR(statbuf.st_mode)) {     // is DIR
        std::string new_full_path = index_file_iterator(full_path, location_block->index);
        if (!new_full_path.empty())     // here will server the static files .html
            res.set_path(new_full_path);
        else if (new_full_path.empty() && location_block->autoindex) {
            autoindex_page(full_path, res);
        } else {
            res.set_stat_code(FORBIDDEN_ACCESS); // forbidden access to autoindex html page
        }
    }
}

void response_builder::build_response(client &current_client, std::deque<ServerBlock> &config)
{
    /*
        TODO: just in case the request is ready && no error in the request, if not
            will serve the error page.
    */

    // get server block match
    int s_host = address_resolution(host);
    const ServerBlock *server_req = getServerForRequest(s_host, port, config);
    if (server_req != NULL) {
        const LocationBlock *getLocatoin = getLocation(path, *server_req);   // get location
        if (getLocatoin != NULL) // find location
        {
            if (is_allowd_method(getLocatoin->allow_methods, method)) {
                path_validation(getLocatoin, current_client.res);
            } else {
                current_client.res.set_stat_code(METHOD_NOT_ALLOWED);
            }
        } else
            current_client.res.set_stat_code(NOT_FOUND);
    }
    else
        current_client.res.set_stat_code(NOT_FOUND);
}

// socket_engine::precccess_connection file:
# include "../socket_engine.hpp"
# include "../response.hpp"
# include "../response_builder.hpp"

void socket_engine::process_connections(void)
{

    std::deque<ServerBlock> server_config_info = get_server_config_info();

    while (true) {
        int epoll_stat = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);
        for (int i = 0; i < epoll_stat; i++)
        {
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
                        std::cout << "[!] accept error: "<< strerror(errno) << std::endl;
                }
            }
            else    // client event -> done[]
            {
                std::cout << "[>] Data incoming from Client FD: " << fd << std::endl;

                char raw_data[BUFFER_SIZE];
                std::memset(raw_data, 0, sizeof(raw_data));

                int recv_stat = recv(fd, raw_data, BUFFER_SIZE, 0);
                std::cout << "Received " << recv_stat << " bytes from fd " << fd << std::endl;
                
                if (recv_stat > 0) {

                    std::cout << "[>] Received " << recv_stat << " bytes from fd " << fd << std::endl;
                    std::cout << "[>] --- DATA START ---\n" << raw_data << "\n--- DATA END ---" << std::endl;
                    
                    // TODO: REQ/RES
                    // raw_client_data[fd].req;

                    // if (raw_client_data[fd].req_ready) {
                        std::cout << ">>>>>>>>>>> ENTTTERRRRR <<<<<<<<<<<<" << std::endl;
                        response_builder response_builder;
                        response_builder.build_response(raw_client_data[fd], server_config_info);
                        std::cout << ">>>>>>>>>> STATUS CODE: " << raw_client_data[fd].res.get_stat_code() << " <<<<<<<<<<" << std::endl;
                    // }
                    // ------------------------------------------------------------------------------------------ //
                }
                else {
                    close (fd);
                    raw_client_data.erase(fd);
                    remove_fd_from_list(fd);
                    if (recv_stat == 0)  // EOF
                        std::cerr << "[!] Client lost connection" << std::endl;
                    else
                        std::cerr << "[!] Client connection broke: " << strerror(errno) << std::endl;
                }

            }
        }
    }
}


