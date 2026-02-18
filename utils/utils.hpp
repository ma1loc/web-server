# ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sstream>

const std::string   to_string(int num);
const std::string   &stat_code_to_string(unsigned short int stat_code);
const std::string   file_to_string(const std::string& path);

# endif