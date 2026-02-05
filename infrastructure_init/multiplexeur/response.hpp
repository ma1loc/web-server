# ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <map>
# include <string>

// start line 'http', 1.0

class response
{
    private:
        unsigned short int stat_code;
        std::map<int, std::string> stat_code_msg;
        std::map<std::string, std::string> header;
        std::string body;
        std::string raw_response;
    public:
        response();
};

# endif