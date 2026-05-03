#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <vector>

/**
 * toggle_nonblocking:
 * Uses fcntl to flip the O_NONBLOCK flag on any File Descriptor.
 */
void toggle_nonblocking(int fd, bool enable) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return;
    
    if (enable)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
        
    fcntl(fd, F_SETFL, flags);
}

int main(int argc, char **argv) {
    bool use_nonblock = (argc > 1); // Turn on if any arg is passed
    int pipe_fds[2];

    if (pipe(pipe_fds) == -1) return 1;

    if (use_nonblock) {
        std::cout << "[+] Mode: NON-BLOCKING\n";
        toggle_nonblocking(pipe_fds[0], true);
        toggle_nonblocking(pipe_fds[1], true);
    } else {
        std::cout << "[!] Mode: BLOCKING (Default)\n";
    }

    pid_t pid = fork();

    if (pid == 0) { // CHILD: The "CGI"
        close(pipe_fds[0]);
        
        // Try to write 100KB (Pipe limit is ~64KB)
        std::vector<char> data(102400, 'A'); 
        std::cout << "[Child] Attempting to write 100KB to pipe...\n";
        
        ssize_t bytes_written = write(pipe_fds[1], data.data(), data.size());
        
        if (bytes_written == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                std::cout << "[Child] Pipe is full! Write would block, so it returned EAGAIN.\n";
            else
                perror("[Child] Write error");
        } else {
            std::cout << "[Child] Wrote " << bytes_written << " bytes.\n";
        }
        close(pipe_fds[1]);
        return 0;

    } else { // PARENT: The "Webserv"
        close(pipe_fds[1]);
        sleep(1); // Wait for child to fill the pipe

        char buf[1024];
        std::cout << "[Parent] Attempting to read from pipe...\n";
        
        ssize_t bytes_read = read(pipe_fds[0], buf, sizeof(buf));

        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                std::cout << "[Parent] No data yet! Read would block, so it returned EAGAIN.\n";
            else
                perror("[Parent] Read error");
        } else {
            std::cout << "[Parent] Read " << bytes_read << " bytes.\n";
        }
        close(pipe_fds[0]);
    }
    return 0;
}