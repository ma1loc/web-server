# include "../socket_engine.hpp"

socket_engine::socket_engine()
{
    /*  epoll_create()
        epoll_create created a table in the kernal level to save the socket fds
    */
    epoll_fd = epoll_create(1);
    if (epoll_fd < 0)
    {
        std::cerr << "[-] Error epoll_create failed: " << strerror(errno) << std::endl; // ERR
        exit(1);
    }
}

void socket_engine::set_fds_list(int fd) {
    fds_list.push_back(fd);
}

void socket_engine::remove_fd_from_list(int fd)
{
    // rm-me
    std::cout << "--------------------------------------------------------------" << std::endl;
    std::cout << "[>] list befor" << std::endl;
    for (size_t i = 0; i < fds_list.size(); i++)
    {
        std::cout << "fd=" << fds_list.at(i) << " ";
    }
    std::cout << "\n";
    // rm-me

    std::vector<int>::iterator fd_position = std::find(fds_list.begin(), fds_list.end(), fd);
    if (fd_position != fds_list.end())
        fds_list.erase(fd_position);

    // rm-me
    std::cout << "[>] list after" << std::endl;
    for (size_t i = 0; i < fds_list.size(); i++)
    {
        std::cout << "fd=" << fds_list.at(i) << " ";
    }
    std::cout << "\n";
    std::cout << "--------------------------------------------------------------" << std::endl;
    // rm-me
}

void socket_engine::free_fds_list(void)
{
    for (unsigned long i = 0; i < fds_list.size(); i++)
    {
        close (fds_list.at(i));
        std::cout << ">>> free fd[" << fds_list.at(i) << "]" << std::endl;  // rm-me
    }
    std::cout << ">>> free fd[" << epoll_fd << "]" << std::endl;  // rm-me
    close (epoll_fd);
}

void socket_engine::set_server_side_fds(int s_fd) {
    server_side_fds.push_back(s_fd);
}

std::vector<int> socket_engine::get_server_side_fds(void) const {
    return (server_side_fds);
}

const std::deque<ServerBlock> &socket_engine::get_server_config_info() const {
    return (server_config_info);
}

void socket_engine::set_server_config_info(std::deque<ServerBlock> server_config_info)
{
    this->server_config_info = server_config_info;
}

// --------------------------------------------------------------------------------------------------------------

void socket_engine::check_all_client_timeouts(void)
{
    time_t now = time(0);
    std::map<int, Client>::iterator it = raw_client_data.begin();

    std::cout << GREEN_S << "NUMBER of the clients -> " << raw_client_data.size() << GREEN_E << std::endl;

    while (it != raw_client_data.end())
    {
        size_t timeout_limit = TIMEOUT_LIMIT;
        int fd = it->first;

        if (it->second.server_conf != NULL)
            timeout_limit = it->second.server_conf->set_timeout;

        if (it->second.close_connection)
        {
            // Clean up any active CGI output pipes for this client before erasing it.
            // Without this, stale pipe_to_client entries survive, get routed to whatever
            // new client reuses the same fd number, and checkResponseAndTime() calls
            // waitpid(0,...) (pid uninitialised on new client) which accidentally reaps
            // other live CGI children.
            for (std::map<int,int>::iterator p = pipe_to_client.begin(); p != pipe_to_client.end(); )
            {
                if (p->second == fd) {
                    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, p->first, NULL);
                    remove_fd_from_list(p->first);
                    close(p->first);
                    pipe_to_client.erase(p++);
                } else
                    ++p;
            }
            for (std::map<int,int>::iterator p = pipe_write_to_client.begin(); p != pipe_write_to_client.end(); )
            {
                if (p->second == fd) {
                    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, p->first, NULL);
                    remove_fd_from_list(p->first);
                    close(p->first);
                    pipe_write_to_client.erase(p++);
                } else
                    ++p;
            }
            if (it->second.cgiHandler.state == CGI_READING || it->second.cgiHandler.state == CGI_WAITING)
                kill(it->second.cgiHandler.pid, SIGTERM);
            epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
            raw_client_data.erase(it++);
            remove_fd_from_list(fd);
            close(fd);
            continue;
        }

        if ((now - it->second.last_activity) > (time_t)timeout_limit && !it->second.close_connection)
        {
            // exit(123);
			bool active_pipe_write = false;
			for (std::map<int,int>::iterator p = pipe_write_to_client.begin(); p != pipe_write_to_client.end(); ++p)
            {
				if (p->second == fd)
                {
                    active_pipe_write = true;
                    break;
                }
			}
			if (active_pipe_write) { ++it; continue; }
            // Skip timeout when client is still waiting for CGI output
            bool active_pipe_read = false;
            for (std::map<int,int>::iterator p = pipe_to_client.begin(); p != pipe_to_client.end(); ++p)
            {
                if (p->second == fd) {
                    active_pipe_read = true;
                    break;
                }
            }
            if (active_pipe_read) { ++it; continue; }
            // >>>>>>>>>> ADD THIS BLOCK HERE <<<<<<<<
            for (std::map<int,int>::iterator p = pipe_to_client.begin(); p != pipe_to_client.end(); )
            {
                if (p->second == fd)  // this pipe belongs to the client we're about to erase
                {
                    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, p->first, NULL);
                    remove_fd_from_list(p->first);
                    kill(it->second.cgiHandler.pid, SIGTERM);
					pipe_to_client.erase(p++);
                }
                else
                    ++p;
            }
            // >>>>>>>>>> END OF ADDED BLOCK <<<<<<<<

			for (std::map<int,int>::iterator p = pipe_write_to_client.begin();
				p != pipe_write_to_client.end(); )
			{
				if (p->second == fd) {
					epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, p->first, NULL);
					remove_fd_from_list(p->first);
					close(p->first);
					pipe_write_to_client.erase(p++);
				} else
					++p;
			}

            epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
            raw_client_data.erase(it++);
            remove_fd_from_list(fd);
            close(fd);
            continue;
        }
        else
            ++it;
    }
}

// --------------------------------------------------------------------------------------------------------------

void    socket_engine::terminate_client(int fd, std::string stat)
{
    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    this->raw_client_data.erase(fd);
    remove_fd_from_list(fd);
    close(fd);

    std::cerr << GREEN_S << stat << GREEN_E << std::endl;
}

void    socket_engine::modify_epoll_event(ssize_t fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    std::cout << "[DEBUG] modify_epoll_event: fd=" << fd << " events=" << events << std::endl;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
    if (ret == -1) {
        // If MOD fails, try ADD (in case FD was removed or doesn't exist yet)
        std::cout << "[DEBUG] MOD failed, trying ADD" << std::endl;
        ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
        if (ret == -1)
            std::cerr << "[!] epoll_ctl ADD also failed: " << strerror(errno) << std::endl;
    }
}
