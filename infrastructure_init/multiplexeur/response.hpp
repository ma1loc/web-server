# ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <map>
# include <string>
# include "../../config_parsing/ConfigPars.hpp"

struct client;

class response
{
    private:
        unsigned short int stat_code;
        std::map<int, std::string> stat_code_msg;

        std::map<std::string, std::string> header;
        std::string body;
    public:
        response();
        // void set_stat_code(unsigned short int stat_code);
};

void    response_handler(std::deque<ServerBlock> &server_config_info, client &current_client);

// void    response::set_stat_code(unsigned short int stat_code)
// {
//     this->stat_code = stat_code;
// }

# endif