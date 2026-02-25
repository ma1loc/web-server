# ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sstream>

const std::string   to_string(int num);
const std::string   file_to_string(const std::string& path);
const std::string   &stat_code_to_string(unsigned short int stat_code);
const std::string   resolved_path_extension(std::string path);
std::string         get_time();
bool                is_valid_error_path(std::string path);

# endif