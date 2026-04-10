#include "Session.hpp"

Session::Session() : id(""), last_access(0) {}

Session& Session::operator=(const Session& obj)
{
    if (this != &obj)
    {
        id = obj.id;
        data = obj.data;
        last_access = obj.last_access;
    }
    return *this;
}

Session::Session(const Session& obj)
{
    id = obj.id;
    data = obj.data;
    last_access = obj.last_access;
}

std::string& Session::GetSessionVal(std::string key)
{
    return data[key];
}

void Session::SetSessionVal(std::string key, std::string value)
{
    data[key] = value;
}

Session::~Session() {}