# include "../response_builder.hpp"
# include "../socket_engine.hpp"    // used just to indelucde the client struct //  rm-me
# include "../utils/utils.hpp"

bool    is_dir_exist(const std::string &path)
{
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {

        return (true);
    }
    return (false);
}

// std::string    response_builder::get_stat_code_path(unsigned int stat_code)
// {
//     std::deque<int> key;

//     key.push_back(stat_code);
//     std::map<std::deque<int>, std::string>::const_iterator it = this->current_client->server_conf->error_page.find(key);
//     if(it != this->current_client->server_conf->error_page.end())
//         return (it->second);
//     return ("");
// }

std::string    response_builder::get_stat_code_path(unsigned int stat_code)
{
    std::map<int, std::string>::const_iterator it = this->current_client->server_conf->error_page.find(stat_code);
    if(it != this->current_client->server_conf->error_page.end())
        return (it->second);
    return ("");
}


std::string join_root_path(const std::string root, std::string path)
{
    std::string final_url = root;
    if (!final_url.empty() && final_url.at(final_url.length() - 1) == '/')
        final_url.erase(final_url.length() - 1);
    
    if (!path.empty() && path.at(0) != '/')
        final_url += "/";
    
    final_url += path;
    normalisePath(final_url, "//", "/", 2);
    normalisePath(final_url, "/./", "/", 3);
    return (final_url);
}

void    response_builder::path_validation()
{
    std::string index;
    struct stat statbuf;

    // ----------------------------------------
    // if (current_client->server_conf == NULL)
    //     exit(100);
    // if (current_client->location_conf == NULL)
    //     exit(200);
    // ----------------------------------------

    std::string req_path = this->current_client->req.getPath();
    const std::string &loc_path = current_client->location_conf->path;
    if (req_path.size() >= loc_path.size() && req_path.substr(0, loc_path.size()) == loc_path)
        req_path = req_path.substr(loc_path.size());
    
    this->path = join_root_path(current_client->location_conf->root, req_path);
    if (stat(path.c_str(), &statbuf) < 0) {
        this->current_client->res.set_stat_code(NOT_FOUND);
        return ;
    } else if (access(path.c_str(), R_OK) < 0)  {
        this->current_client->res.set_stat_code(FORBIDDEN_ACCESS);
        return ;
    }
    if (S_ISDIR(statbuf.st_mode))
    {   
        std::cout << "[+] DIR REQUESTED HEEEEEEEEEEEEEREEEEEEEEEE" << std::endl;
        std::cout << "this->path -> " << this->path << std::endl;

        index = index_file_iterator(this->path);
        std::cout << "INDEX -> " << index << std::endl;
        if (!index.empty())     // here will server the static files .html
            this->path = index;
        else if (index.empty() && current_client->location_conf->autoindex)
            autoindex_page(this->path, this->current_client->req.getPath());
        else {
            this->current_client->res.set_stat_code(FORBIDDEN_ACCESS);
        }
    }
}
