# include "../response_builder.hpp"
# include "../socket_engine.hpp"    // used just to indelucde the client struct //  rm-me

std::string    response_builder::get_stat_code_path(unsigned int stat_code)
{
    std::deque<int> key;

    key.push_back(stat_code);
    std::map<std::deque<int>, std::string>::const_iterator it = server_conf->error_page.find(key);
    if(it != server_conf->error_page.end())
        return (it->second);
    return ("");
}

std::string path_normalize(const std::string root, std::vector<std::string> path_holder)
{
    std::string final_url = root;
    for (size_t i = 0; i < path_holder.size(); i++) {
        if (final_url.at(final_url.length() -1) != '/')
            final_url.append("/");
        final_url.append(path_holder.at(i));
    }
    std::cout << ">>> request path afterrrrrrrrrrrrrrrrrr >>> " << final_url << std::endl;
    return (final_url);
}

bool    response_builder::path_resolver()   // WOKING ON IT []
{
    std::string root = this->locatoin_conf->root;
    std::string request_path = PATH0;   // <<<<< PATH TEST

    // rm-me
    std::cout << ">>> root path >>> " << root << std::endl;
    std::cout << ">>> request path beforrrrrrrrrrrrrrrrr >>> " << request_path << std::endl;
    //

    std::vector<std::string> path_holder;
    size_t      start = 0;
    size_t      end;


    while ((end = request_path.find("/", start)) != std::string::npos)
    {
        std::string segment = request_path.substr(start, (end - start));
        if (segment == ".." && path_holder.empty()) {
            this->current_client->res.set_stat_code(FORBIDDEN_ACCESS);
            return (false);
        }
        else if (segment == "..")
            path_holder.pop_back();
        else if (!segment.empty())
            path_holder.push_back(segment);
        start = end + 1;
    }
    path_holder.push_back(request_path.substr(start));

    // ---------------------------------------------------------------------
    // this->current_client->res.set_path(path_normalize(root, path_holder));
    this->path = path_normalize(root, path_holder);
    // ---------------------------------------------------------------------
    
    return (true);
}

void    response_builder::path_validation()
{
    
    if (!path_resolver()) {
        std::cerr << "[!] path_resolver() is failed" << std::endl;
        return ;
    }

    struct stat statbuf;
    std::string index;

    // ----------------- HARDCODED -------------------
    // this->path = this->current_client->res.get_path();
    // -----------------------------------------------

    if (stat(path.c_str(), &statbuf) < 0) {
        this->current_client->res.set_stat_code(NOT_FOUND);
        return ;
    } else if (access(path.c_str(), R_OK) < 0)  {
        this->current_client->res.set_stat_code(FORBIDDEN_ACCESS);
        return ;
    }

    if (S_ISDIR(statbuf.st_mode)) {     // is DIR
        std::cout << "[+] DDDDDDDDDDDDDDDDDDDIR IS HERE" << std::endl;
        index = index_file_iterator(path);
        if (!index.empty())     // here will server the static files .html
            this->path = index;
        else if (index.empty() && this->locatoin_conf->autoindex)
            autoindex_page(this->path);   // STOP HERE //
        else {
            this->current_client->res.set_stat_code(FORBIDDEN_ACCESS);
        }
    }
}