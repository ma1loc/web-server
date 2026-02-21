# include "../response_builder.hpp"
# include "../socket_engine.hpp"    // used just to indelucde the client struct //  rm-me

# include <stack>

// REQUEST HARDCODED VALUES TO TEST
std::string _method_ = "GET";
std::string _protocol_ = "HTTP/1.0";

// ------------------------------- //  rm-me
// std::string _host_ = "localhost";
// std::string _host_ = "192.168.122.1";
std::string _host_ = "10.11.11.4";
int _port_ = 8080;
// ------------------------------- //  rm-me

response_builder::response_builder(): is_body_ready(false) {};

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

    for (size_t i = 0; i < this->locatoin_conf->index.size(); i++) {
        redirection_path = based_path + this->locatoin_conf->index.at(i);
        if (access(redirection_path.c_str(), F_OK | R_OK) == 0)
            return (redirection_path);
    }
    return ("");
}

// // ------------ TEST -------------
// # define METHODE "GET"
// # define METHODE "POST"
// # define METHODE "DELETE"
// // -------------------------------

void    response_builder::handle_post() {
    
}

void response_builder::response_setup()
{
    // ----------- HARDCODED -----------
    std::string _method_ = "GET";
    // std::string _method_ = "POST";
    // std::string _method_ = "DELETE";
    // ---------------------------------

    if (current_client->res.get_stat_code() != OK) {
        generate_error_page();
        return;
    }

    if (_method_ == "GET") {
        handle_get();
    }
    else if (_method_ == "POST") {
        handle_post();
    }
    else if (_method_ == "DELETE") {
        handle_delete();
    }
    this->current_client->res.set_raw_response(response_holder);
}

std::string    response_builder::get_stat_code_path(unsigned int stat_code)
{
    std::deque<int> key;

    key.push_back(stat_code);
    std::map<std::deque<int>, std::string>::const_iterator it = server_conf->error_page.find(key);
    if(it != server_conf->error_page.end())
        return (it->second);
    return (NULL);
}

void response_builder::build_response(client &current_client, std::deque<ServerBlock> &config)
{
    this->current_client = &current_client;

    int s_host = address_resolution(_host_);
    server_conf = getServerForRequest(s_host, _port_, config);  // move

    std::cout << "error path -> " << get_stat_code_path(404) << std::endl;
    exit(123);

    if (server_conf == NULL)
        current_client.res.set_stat_code(NOT_FOUND);
    else {
        // TODO: need to normalize the path before check
        locatoin_conf = getLocation(PATH0, *server_conf);   // <<<<< PATH TEST
        if (locatoin_conf == NULL) { // find location
            current_client.res.set_stat_code(NOT_FOUND);
        }
        else {
            if (is_allowd_method(_method_))
                path_validation();
            else
                current_client.res.set_stat_code(METHOD_NOT_ALLOWED);
        }
    }
    response_setup();
}
