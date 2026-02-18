# ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <map>
# include <string>
# include <vector>
# include "config_parsing/ConfigPars.hpp"

# define OK 200
# define FORBIDDEN_ACCESS 403
# define NOT_FOUND 404
# define METHOD_NOT_ALLOWED 405
# define PAYLOAD_TOO_LARGE 413
# define SERVER_ERROR 500
# define METHOD_NOT_IMPLEMENTED 501

# define PROTOCOL_VERSION   "HTTP/1.0"

class response  // DONE[]
{
    private:
        std::map<std::string, std::string>  header;
        std::string                         body;

        std::string         path;
        unsigned short int  stat_code;
        bool                is_body_ready;
        
        //  USED IN THE HEADER RESPONSE
        std::string         content_type;
        ssize_t             content_length;

    public:
        response();

        // SETTERS
        void    set_stat_code(unsigned short int stat_code);
        void    set_body_contnet(std::string body);
        void    set_path(std::string path);
        void    set_body_as_ready(void);

        // GETTERS
        std::string         get_str_stat_code(unsigned short int code);
        unsigned short int  get_stat_code(void);
        ssize_t             get_content_length(void);
        std::string         get_path(void);
        bool                get_is_body_ready(void);

        // -------------------------------------------------- //

        std::string         get_start_line(void);

};

# endif
