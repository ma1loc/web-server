to done
-> using addrinfo insted of sockaddr_in
-> Virtual Hosting, not req?



subject say:
>>> However, you are allowed to use fcntl() only with the following flags:
F_SETFL, O_NONBLOCK and, FD_CLOEXEC


pollfd = you manage the list
epoll_event = kernel manages the list

In poll(), even if you have 1,000 clients and only one person sends a message, your code has to check 1,000 if statements.

In epoll, the inner loop only runs for the number of people who actually did something. If 1,000 are connected but only one talks, the inner loop runs exactly once.