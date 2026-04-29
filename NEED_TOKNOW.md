>> NEED-TO-KNOW:
    - what is sockets ???
        answer -> socket is a pipeline between server/client and it behave as full-duplix
            but the http protocol make it behave as half-duplix, becouse the http based on the request then response based on the request
            that why the server can not send anything intell the client send a request.

    - what is epoll (epoll_create/ epoll_wait/ epoll_event) ???
    - what is multiplexer ???
    - how http works ???
    - how get/post/delete works ???
    - how sockets works ???
    - what is session ???
    - what diff between sockets/session ???
    - how to sockets connect to each other (Client Socket <> Server Socket) ???
    - what's the diff between tcp/udp sockets


504 status code (gateway timeout)

---------------------------------------------
- EPOLLRDHUP && EPOLLHUP
    if (events & (EPOLLHUP | EPOLLERR)) {   // RST -> close after (T-W H)
        terminate_client(fd, "[-] RST: network connection dropped/Error ");
        return ;
    }
    WHY ????
        if (events & EPOLLRDHUP) {    // 'FIN'
        if (!(events & EPOLLIN)) {    // 'FIN' & 'no data'
            terminate_client(fd, "[-] Client closed connection");
            return ;
        }
    }

- recv return
    else if (recv_stat == 0)
        terminate_client(fd, "[!] Client lost connection (EOF)");
    else
        terminate_client(fd, "[!] Client connection broke");

- class response_builder

- level trigerd
    edge trigerd

- why using the O_NONBLOCK in the case of the pipes && and sockets not:
    >> first thing first sockets is based of TCP Flow
    TCP stack handles the flow control, it slows down the transfer

    >> pipes has a limited buffer size to hold unreaded data around 64kb
---------------------------------------------