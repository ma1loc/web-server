# include "response.hpp"
# include "../socket_engine.hpp"
# include "../../config_parsing/ConfigPars.hpp"

void    response_handler(std::deque<ServerBlock> &server_config_info, client &client)
{
    (void)client;
    (void)server_config_info;

    // REQUEST HARDCODED VALUES TO TEST
    std::string methode = "GET";
    std::string path = "/tours";
    std::string protocol = "HTTP/1.1";
    std::string host = "localhost";
    int port = 8080;

    const ServerBlock *server_req = getServerForRequest(host, port, server_config_info); // get server block
    if (server_req != NULL) {
        std::cout << "VALID REQUEST 200, OK" << std::endl;

        const LocationBlock *is_getLocatoin = getLocation(path, *server_req);   // get location
        std::cout << "is_getLocatoin->path: " << is_getLocatoin->path << std::endl;
        if (is_getLocatoin != NULL)
        {
            std::cout << "index size: " << is_getLocatoin->index.size() << std::endl;
            std::cout << "seccussfully get INDEX PATH: " << is_getLocatoin->index[0] << std::endl;
        } else {
            std::cout << "NO loction is match :)" << std::endl;
        }
    }
    else {
        // client.res.set_stat_code(404);
        std::cout << "BAD REQUEST 404, NOT FOUND" << std::endl;
    }


    // std::cout << "-----------------------------------------------------------" << std::endl;
    // std::cout << "server numbers: " << server_config_info.size() << std::endl;
    
    // std::cout << "host of server 1 " << server_config_info[0].host << std::endl;
    // std::cout << "port of server 1 " << server_config_info[0].listen << std::endl;
    // std::cout << "-----------------------------------------------------------" << std::endl;


}
