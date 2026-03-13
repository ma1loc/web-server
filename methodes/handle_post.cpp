# include "../response_builder.hpp"
# include "../utils/utils.hpp"

std::string extracting_file_name(const std::map<std::string, std::string> &header)
{
    std::string file_name = extracting_from_header(header, "FILENAME");
    if (file_name.empty())  // No file_name
    {
        std::string content_type;
        file_name = rand_str_gen();
        content_type = extracting_from_header(header, "CONTENT_TYPE");
        if (content_type.empty())   // No Content-type
            file_name += DEFAULT_EXTENSION;
        else                        // There's Content-type
            file_name += media_type_to_extension(content_type);
    }
    return (file_name);
}



std::string validate_upload_path(Client &current_client)
{
    std::string file_name = extracting_file_name(current_client.req.getHeaders());
    std::string file_path;
    file_path = join_root_path(current_client.location_conf->root, current_client.req.getPath());
    if (!is_dir_exist(file_path)) {
        current_client.res.set_stat_code(NOT_FOUND);
        return ("");
    }
    if (access(file_path.c_str(), X_OK | W_OK) < 0) {
        current_client.res.set_stat_code(FORBIDDEN_ACCESS);
        return ("");
    }

    file_path = join_root_path(file_path, file_name);
    
    int file_fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        current_client.res.set_stat_code(SERVER_ERROR);
        return ("");
    }
    current_client.res.set_static_file_fd(file_fd);
    return (file_path);
}



void    response_builder::handle_post()
{
    std::string file_name = validate_upload_path(*this->current_client);
    if (file_name.empty()) {
        generate_error_page();
        return ;
    }


    // rm-me
    std::cout << "[>] extracting_file_name -> " << file_name << std::endl;
    std::cout << "[>] file FD -> " << this->current_client->res.get_static_file_fd() << std::endl;
    // exit(88);

    // >>>>>>>>>>>>>>>>>>>>>>>>> Body Processing >>>>>>>>>>>>>>>>>>>>>>>>>
    
    // is have alrady the body ready to make a response based on it
    const std::string &body_buff = this->current_client->req.getBody();
    


    int short write_stat = write(this->current_client->res.get_static_file_fd(), body_buff.c_str(), body_buff.size());
    if (write_stat < 0) {
        close (this->current_client->res.get_static_file_fd());
        unlink(file_name.c_str());
        this->current_client->res.set_stat_code(SERVER_ERROR);
        return ;
    }
    close (this->current_client->res.get_static_file_fd());
    this->current_client->res.set_stat_code(CREATED);

    response_holder.append(current_client->res.get_start_line());
    response_holder.append("Server: Webserv\r\n");
    response_holder.append("Date: " + get_time() + "\r\n");

    std::cout << "++++ [>] POST STATUS CODE " << current_client->res.get_stat_code() << std::endl;

}
