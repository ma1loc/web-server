# ifndef RESPONSE_BUILDER_HPP
# define RESPONSE_BUILDER_HPP

# include <deque>
# include <vector>
# include <dirent.h>
# include <sys/stat.h>
# include "response.hpp"
# include "./config_parsing/ConfigPars.hpp"

struct client;

class response_builder
{
    private:
        client                  *current_client;
        const ServerBlock       *server_conf;
        const LocationBlock     *locatoin_conf;

        bool        is_allowd_method(std::string method);
        // bool        is_allowd_method(std::deque<std::string> allow_methods, std::string method);

        void        path_validation();
        // void        path_validation(const LocationBlock *location_block, response &res);
        
        std::string path_resolver();
        // std::string path_resolver(std::string root, std::string path);
        
        std::string index_file_iterator(const std::string &full_path);
        // std::string index_file_iterator(std::string &full_path, const std::deque<std::string> &index);
        
        std::string html_index_gen(std::vector<std::string> &dir_list, const std::string &uri_path);
        
        void        autoindex_page(const std::string &full_path);
        // void        autoindex_page(std::string &full_path, response &res);

        
        // to-do think about
        void        prepare_content_source(client &current_client, unsigned short int stat_code);
        void        serve_static_file(client &current_client);  // to-do
    public:
        response_builder();
        void build_response(client &current_client, std::deque<ServerBlock> &config);
};

# endif
