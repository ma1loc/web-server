# include "utils.hpp"

const std::string   &to_string(int num)
{
    std::stringstream str;
    str << num;
    return (str.str());
}

