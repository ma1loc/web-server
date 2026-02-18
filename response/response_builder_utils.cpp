# include "../response_builder.hpp"
# include "../socket_engine.hpp"    // use to use client struct

/*  TODO:
    in the case of autoindex user has limit access to files (../..) []
    user root path (~/) -> (web-serve dir) []
    serve_static_file() based on:
        if -> status_code not 2** OK:
            serve error page
            if -> there's no error page in the local
                will generate on with the status code anyway  
        else -> serve the html file request
            if -> file exist
                serve the file
            else -> serve the error page
                if -> the error page not eixst
                    will create on as i say anyway
                else -> read the .html file and serve it
        if -> the request is a dir
            will check the index files in the config file loop, intel get a valid file
            else -> all the files in index config file is not exist or have no permission to read it
                as the request is dir will check the autoindex is on or off
                if -> autoindex is on
                    will gen a index of (****) page
                else -> autoindex is off
                    will throw an 403 Forbidden
        will check the edge case too like:
            huge payload > client_max_body_size
            autoindex vulnerable by request like "../../.. ..etc" intel access somthing out of scope
    ------------------------------------------------------------
    before calling the serve_static_file(); -> i alrady has
    body: is alrady ready -> if the autoindex detect i set the body and set it as ready 
    path: in case of a valid file to serve it, (eixst, has R permission)
    payload max check will be in case has a huge data
*/

void        response_builder::serve_static_file()
{
    
}
