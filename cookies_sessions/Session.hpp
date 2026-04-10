#ifndef SESSION_HPP
#define SESSION_HPP

#include <iostream>
#include <map>
#include <ctime>

class Session
{
    public:
        std::string id;
        std::map<std::string, std::string> data;
        time_t last_access;
        Session();
        Session(const Session& obj);
        void SetSessionVal(std::string key, std::string value);
        std::string& GetSessionVal(std::string key);
        Session& operator=(const Session& obj);
        ~Session();
};



#endif