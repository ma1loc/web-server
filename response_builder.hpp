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
        void        path_validation();
        bool        path_resolver();
        std::string index_file_iterator(const std::string &full_path);
        void        autoindex_gen(std::vector<std::string> &dir_list, const std::string &uri_path);
        void        autoindex_page(const std::string &full_path);
        void        default_error_page(unsigned short int stat_code);


        // to-do think about
        // void        prepare_content_source(unsigned short int stat_code);
        void        serve_static_file();  // to-do
    
    public:
        response_builder();
        void build_response(client &current_client, std::deque<ServerBlock> &config);
};

# endif
