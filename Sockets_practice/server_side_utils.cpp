# include "sockets.hpp"

// I/O Multiplexing
/*  TODO: poll()
    SYNTAX -> int poll(struct pollfd *ufds, unsigned int nfds, int timeout);
    watches many sockets, waits up to timeout, and tells you how many are ready.
    ufds -> array of pollfds i get
    nfds -> sizeof pollfds array
    timeout -> how long poll() waits
*/

void    process_connections()
{
    std::vector<struct pollfd> &fds = s_engine.get_poll_fds();
    while (true)
    {
        int poll_stat = 0;
        poll_stat = poll(fds.data(), fds.size(), TIMEOUT);
        if (poll_stat < 0) {
            std::cerr << "Error: poll field to watch sockets\n" << errno << std::endl;
            std::exit(1);
        }
        for (int i = 0; i < fds.size(), i++)
        {}
    }
}
