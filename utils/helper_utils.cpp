# include "utils.hpp"
# include <map>

# include <iostream>

const std::string   to_string(int num)
{
    std::stringstream str;
    str << num;
    return (str.str());
}

const std::string   &stat_code_to_string(unsigned short int stat_code)
{
    static std::map<int, std::string> stat_code_str;
    if (stat_code_str.empty()) {
        stat_code_str[200] = "OK";
        stat_code_str[403] = "Forbidden";
        stat_code_str[404] = "Not Found";
        stat_code_str[405] = "Method Not Allowed";
        stat_code_str[413] = "Payload Too Large";
        stat_code_str[500] = "Internal Server Error";
    }
    return (stat_code_str[stat_code]);
}
