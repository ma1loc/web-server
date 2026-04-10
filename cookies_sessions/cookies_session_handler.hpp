#ifndef COOKIES_SESSION_HANDLER_HPP
#define COOKIES_SESSION_HANDLER_HPP

#include "Session.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

// cookie parsing
void  parseCookies(std::string header);
std::vector<std::string> split(std::vector<std::string> keywords, char dilimeter);

#endif