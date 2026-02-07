# include "../response.hpp"
# include "../socket_engine.hpp"
# include "../config_parsing/ConfigPars.hpp"

# include <sys/stat.h>

// REQUEST HARDCODED VALUES TO TEST
std::string method = "GET";
std::string path = "/"; // most change this when you request a new path
std::string protocol = "HTTP/1.1";
std::string host = "localhost";
int port = 8080;
// --------------------------------

bool    is_allowd_method(std::deque<std::string> allow_methods, std::string method)
{
    for (size_t i = 0; i < allow_methods.size(); i++) {
        if (allow_methods.at(i) == method)
            return (true);
    }
    return (false);
}

std::string index_file_iterator(std::string &full_path ,const std::deque<std::string> &index)
{
    std::string redirection_path;
    std::string based_path = full_path;
    if (!full_path.empty() && full_path.at(full_path.length() -1) != '/')
        based_path += '/';

    for (size_t i = 0; i < index.size(); i++)
    {
        redirection_path = based_path + index.at(i);
        std::cout << "redirection_path -> " << redirection_path << std::endl;
        if (access(redirection_path.c_str(), R_OK) == 0)
            return (redirection_path);
    }
    return ("");
}

void    autoindex_page(client &current_client) // autoindex: opendir, readdir and closedir.
{
    current_client.res.
}

// in case of the path match the ("request path" == "config path") check in local 
void    path_validation(const LocationBlock *location_block, client &current_client)
{
    struct stat statbuf;
    std::string full_path;
    const std::string &root = location_block->root;
    const std::string &path = location_block->path;

    full_path = root + path;
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>. FULL PATH: " << full_path << std::endl;

    if (stat(full_path.c_str(), &statbuf) < 0 || access(full_path.c_str(), R_OK) < 0) {    // existence
        current_client.res.set_stat_code(NOT_FOUND);
        return ;
    }
    if (S_ISDIR(statbuf.st_mode)) {     // is DIR
        full_path = index_file_iterator(full_path, location_block->index);
        if (full_path.empty() && location_block->autoindex) {
            autoindex_page(current_client);
        } else
            current_client.res.set_stat_code(FORBIDDEN_ACCESS); // forbidden access to autoindex html page
    }
}

void    response_handler(std::deque<ServerBlock> &server_config_info, client &current_client)
{
    // get server block match
    const ServerBlock *server_req = getServerForRequest(host, port, server_config_info);
    if (server_req != NULL) {
        const LocationBlock *getLocatoin = getLocation(path, *server_req);   // get location
        if (getLocatoin != NULL) // find location
        {
            std::cout << ">>>>>>>>>>>>>>>>>>>>>>>. VALID PATH (hardcoded path [" << path << "])" << std::endl;
            // check the method is allowd
            if (is_allowd_method(getLocatoin->allow_methods, method)) {
                // check the path is exist
                path_validation(getLocatoin, current_client);
            } else {
                current_client.res.set_stat_code(METHOD_NOT_ALLOWED);
            }
        } else {    // location not found
            std::cout << ">>>>>>>>>>>>>>>>>>>>>>>. NOT VALID PATH" << std::endl;
            current_client.res.set_stat_code(NOT_FOUND);
        }
    }
    else {
        // client.res.set_stat_code(404);
        std::cout << "<<<<<<<<<<<<<<<<<<<<< BAD REQUEST 404, NOT FOUND >>>>>>>>>>>>>>>>>>>>>>" << std::endl;
    }
}
