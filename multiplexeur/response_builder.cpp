# include "../response_builder.hpp"
# include "../socket_engine.hpp"    // used just to indelucde the client struct //

// REQUEST HARDCODED VALUES TO TEST
std::string _method_ = "GET";
std::string _protocol_ = "HTTP/1.0";
std::string _path_ = "/";
std::string _host_ = "localhost";
int port = 8080;
// ------------------------------- //

response_builder::response_builder() {};

bool    response_builder::is_allowd_method(std::string method)
{
    for (size_t i = 0; i < this->locatoin_conf->allow_methods.size(); i++) {
        if (this->locatoin_conf->allow_methods.at(i) == method)
            return (true);
    }
    return (false);
}

std::string response_builder::index_file_iterator(const std::string &full_path)
{
    std::string redirection_path;
    std::string based_path = full_path;
    if (!full_path.empty() && full_path.at(full_path.length() -1) != '/')
        based_path += '/';

    for (size_t i = 0; i < this->locatoin_conf->index.size(); i++)
    {
        redirection_path = based_path + this->locatoin_conf->index.at(i);
        if (access(redirection_path.c_str(), F_OK | R_OK) == 0)
            return (redirection_path);
    }
    return ("");
}

void    response_builder::autoindex_page(const std::string &full_path)
{
    std::cout << "<<<<<<<<<<<<<< autoindex_page call >>>>>>>>>>>>>>" << std::endl;

    DIR *dir = opendir(full_path.c_str());
    if (dir == NULL)
        return (this->current_client->res.set_stat_code(FORBIDDEN_ACCESS), (void)0);
    std::vector<std::string> dir_list;
    while (true)
    {
        dirent *read_dir = readdir(dir);
        if (read_dir == NULL)
            break;
        dir_list.push_back(read_dir->d_name);
    }
    std::string html_gen = default_index_page(dir_list, full_path);
    this->current_client->res.set_body_contnet(html_gen);
    this->current_client->res.set_body_as_ready();
    closedir(dir);

    // rm-me
    std::cout << "-- START (autoindex_page) --\n" << html_gen << "\n-- END (autoindex_page)  --\n" << std::endl;
}

std::string response_builder::path_resolver()
{
    std::string root_path = this->locatoin_conf->root;
    if (!root_path.empty() && root_path.at(root_path.length() - 1) == '/')
        root_path.erase(root_path.length() - 1);
    return (root_path + _path_);
}

void    response_builder::path_validation()
{
    struct stat statbuf;

    std::string full_path = path_resolver();
    if (stat(full_path.c_str(), &statbuf) < 0 || access(full_path.c_str(), R_OK) < 0)
        return (this->current_client->res.set_stat_code(NOT_FOUND), (void)0);
    if (S_ISDIR(statbuf.st_mode)) {     // is DIR
        std::string new_full_path = index_file_iterator(full_path);
        if (!new_full_path.empty())     // here will server the static files .html
            this->current_client->res.set_path(new_full_path);
        else if (new_full_path.empty() && this->locatoin_conf->autoindex) {
            autoindex_page(full_path);
        } else {
            this->current_client->res.set_stat_code(FORBIDDEN_ACCESS);
        }
    }
}

void response_builder::build_response(client &current_client, std::deque<ServerBlock> &config)
{
    /*
        TODO: just in case the request is ready && no error in the request, if not
            will serve the error page.
    */

    this->current_client = &current_client;
    int s_host = address_resolution(_host_);
    server_conf = getServerForRequest(s_host, port, config);
    if (server_conf != NULL) {
        locatoin_conf = getLocation(_path_, *server_conf);   // get location
        if (locatoin_conf != NULL) // find location
        {
            if (is_allowd_method(_method_)) {
                path_validation();
            } else {
                current_client.res.set_stat_code(METHOD_NOT_ALLOWED);
            }
        } else
            current_client.res.set_stat_code(NOT_FOUND);
    }
    else
        current_client.res.set_stat_code(NOT_FOUND);
    // serve_static_file();
}
