# include "response.hpp"

response::response() {
    stat_code = 200;

    stat_code_msg[200] = "OK";
    stat_code_msg[403] = "Forbidden";
    stat_code_msg[404] = "Not Found";
    stat_code_msg[405] = "Method Not Allowed";
    stat_code_msg[500] = "Internal Server Error";

}

void response::set_stat_code(unsigned short int stat_code) {
    this->stat_code = stat_code;
}

void response::set_start_line() {
    this->start_line = "HTTP/1.0"
}