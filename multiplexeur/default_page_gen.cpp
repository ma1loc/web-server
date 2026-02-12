# include "../response_builder.hpp"
# include "../utils/utils.hpp"

# include "../socket_engine.hpp"    // use to use client struct

void    response_builder::default_error_page(unsigned short int stat_code)
{
    // std::string str_code = to_string(stat_code);
    std::string status_code = to_string(stat_code);
    std::string str_of_stat_code = stat_code_to_string(stat_code);
    std::string title = "Webserver: " + status_code + str_of_stat_code;
    this->current_client->res.set_body_contnet("<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "   <meta charset=\"UTF-8\">\n"
        "   <title>" + title + "</title>\n"
        "</head>\n"
        "<body>\n"
        "   <div class=\"deful_page_container\">\n"
        "       <h1>" + status_code + "</h1>\n"
        "       <p class=\"error_code\">" + str_of_stat_code + "</p>\n"
        "       <p class=\"error_msg\"> The server was unable to complete your request. </p>\n"
        "       <p class=\"go_back\"> <a href=\"/\"> Return to Home </a> </p>\n"
        "   </div>\n"
        "</body>\n"
        "</html>");
    this->current_client->res.set_stat_code(stat_code);
    this->current_client->res.set_body_as_ready();
}

std::string response_builder::default_index_page(std::vector<std::string> &dir_list, const std::string &uri_path)
{
    std::string gen_html = "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "   <meta charset=\"UTF-8\">\n"
        "   <title> Index of " + uri_path + "</title>\n"
        "</head>\n"
        "<body>\n";

    gen_html.append("<a href=\"../\">../</a><br>\n");

    for (size_t i = 0; i < dir_list.size(); i++) {
        std::string &name = dir_list.at(i);

        if (name == ".") continue;
        if (name == "..") continue;

        gen_html.append("<a href=\"" + name + "\">" + name + "</a><br>\n");
    }
    gen_html.append("<hr>\n</body>\n</html>");
    return (gen_html);
}
