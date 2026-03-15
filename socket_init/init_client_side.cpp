# include "../socket_engine.hpp"

// (DONE[*])
void socket_engine::init_client_side(int fd)
{
    inisializeClient(this->raw_client_data[fd]);
    this->raw_client_data[fd].is_serving_file = false;
    // this->raw_client_data[fd].static_file_fd = -1;
    this->raw_client_data[fd].last_activity = time(0);
    this->raw_client_data[fd].close_connection = false;

    struct epoll_event ev;

    std::memset(&ev, 0, sizeof(ev));
    ev.data.fd = fd;
    /*
        EPOLLIN -> Tell me when ready to read data from FD
        EPOLLRDHUP -> Remote Device Hang Up ()
    */
    ev.events = EPOLLIN | EPOLLRDHUP;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        close (fd);
        std::cerr << "[!] epoll_ctl failed: " << strerror(errno) << std::endl;
        return ;
    }
    set_fds_list(fd);
}

void    socket_engine::add_pipeOut_event(int client_fd, int pipeOut)
{
	struct epoll_event ev;
	ev.events = EPOLLIN;    // ready for a read
	ev.data.fd = pipeOut;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipeOut, &ev);
	this->pipe_to_client[pipeOut] = client_fd;
    set_fds_list(pipeOut);
}

void    socket_engine::add_pipeIn_event(int client_fd, int pipeIn)
{
	struct epoll_event ev;
    ev.events = EPOLLOUT;   // ready for a write
	ev.data.fd = pipeIn;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipeIn, &ev);
	this->pipe_write_to_client[pipeIn] = client_fd;
	set_fds_list(pipeIn);
}

void    socket_engine::setup_cgi_pipes(int fd)
{
    std::cout << "clled just onece CGI" << std::endl;
    // pipe_out_fd -> to read from it
	int pipe_out_fd = this->raw_client_data[fd].cgiHandler.getPipeOutFd();     // pipeOut[0] -> r
    int pipe_in_fd  = this->raw_client_data[fd].cgiHandler.getPipeInFd();   // pipeIn[1]  -> w
	
    // delete the client fd from epoll intel the CGI finish finish response pipeOut[0]
    // epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    // modify_epoll_event(fd, EPOLLOUT);   // just in case there's a data to send
    modify_epoll_event(fd, 0);

    add_pipeOut_event(fd, pipe_out_fd);  // pipeOut[0] -> EPOLLIN  (read CGI output)
	if (this->raw_client_data[fd].parse.body)
		add_pipeIn_event(fd, pipe_in_fd);   // pipeIn[1]  -> EPOLLOUT (write body to CGI)
	else
		close(pipe_in_fd);
}

