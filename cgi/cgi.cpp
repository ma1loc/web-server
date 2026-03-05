#include "cgi.hpp"

Cgi::Cgi()
{
}

Cgi::Cgi(const Cgi &other)
{
    *this = other;
}

Cgi &Cgi::operator=(const Cgi &other)
{
    if (this != &other)
    {
        interpreter = other.interpreter;
        extension   = other.extension;
    }
    return *this;
}

Cgi::~Cgi()
{
}

void Cgi::setInterpreter(const std::string &interpreter)
{
    this->interpreter = interpreter;
}

void Cgi::setExtension(const std::string &extension)
{
    this->extension = extension;
}

std::string Cgi::getInterpreter() const
{
    return interpreter;
}

std::string Cgi::getExtension() const
{
    return extension;
}

bool Cgi::checkForCgi(Client &client)
{
    if (client.location_conf->cgi_handler.empty())
        return false;

    size_t dot = client.req.getPath().rfind('.');

    if (dot == std::string::npos)
        return false;

    std::string exten = client.req.getPath().substr(dot);
    std::map<std::string, std::string>::const_iterator it =
        client.location_conf->cgi_handler.begin();

    for (; it != client.location_conf->cgi_handler.end(); it++)
    {
        if (exten == it->first)
        {
            interpreter = it->second;
            extension   = it->first;
            return true;
        }
    }
    return false;
}