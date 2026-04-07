#include "cgi.hpp"
#include "../client.hpp"
#include "../utils/utils.hpp"
# include <fcntl.h>
#include <algorithm>

// size_t Cgi::CGI_MAX_OUTPUT = 10000000;
size_t Cgi::CGI_MAX_OUTPUT = 200000000;

Cgi::Cgi()
{
    envp = NULL;
    argv = NULL;
	body_bytes_sent = 0;
}


Cgi &Cgi::operator=(const Cgi &other)
{
    if (this != &other)
    {
        interpreter = other.interpreter;
        extension = other.extension;
        body_bytes_sent = other.body_bytes_sent;  // ← ADD THIS
    }
    return *this;
}

Cgi::Cgi(const Cgi &other)
{
    *this = other;
}

Cgi::~Cgi() {}

void Cgi::setInterpreter(const std::string &interpreter)
{
    this->interpreter = interpreter;
}

void Cgi::setExtension(const std::string &extension)
{
    this->extension = extension;
}

std::string Cgi::getInterpreter() const
{
    return interpreter;
}

std::string Cgi::getExtension() const
{
    return extension;
}

char **Cgi::getArgv() const
{
    return argv;
}

char **Cgi::getEnv() const
{
    return envp;
}

std::string Cgi::getResponse() const {
    return response;
}

std::string &Cgi::getResponseRef() {
    return response;
}

void Cgi::checkForCgi(Client &client)
{
    std::cout << "checkForCgi -----------------------------------------------------------" << std::endl;
    std::cout << "client.req.getPath() -> " << client.req.getPath() << std::endl;
    if (client.location_conf->cgi_handler.empty())
    {
        state = CGI_NOT_REQUIRED;
        return;
    }
    size_t dot = client.res.get_path().rfind('.');

    std::cout << "client.res.get_path() -> " << client.res.get_path() << std::endl;
    std::cout << "client.res.get_path().rfind('.') -> " << client.res.get_path().rfind('.') << std::endl;

    if (dot == std::string::npos) {
        state = CGI_NOT_REQUIRED;
        return;
    }

    std::string exten = client.res.get_path().substr(dot);
    std::cout << "client.res.get_path().substr(dot) -> " << client.res.get_path().substr(dot) << std::endl;

    std::map<std::string, std::string>::const_iterator it =
        client.location_conf->cgi_handler.begin();

    for (; it != client.location_conf->cgi_handler.end(); it++)
    {
        // std::cout << "enter" << std::endl;
        if (exten == it->first)
        {
            interpreter = it->second;
            extension = it->first;
            state = SETUP_CGI;

            // std::cout << "it's a cgi" << std::endl;
            // exit(1);

            return;
        }
    }
    state = CGI_NOT_REQUIRED;
    std::cout << "-----------------------------------------------------------" << std::endl;
}

void collectEnv(Client &client, std::vector<std::string> &env)
{
    // ------------------ php used ----------------------------
    env.push_back("REQUEST_METHOD=" + client.req.getMethod());
    env.push_back("QUERY_STRING=" + client.req.getQuery());
    env.push_back("SCRIPT_NAME=" + client.req.getPath());
    // ---------------------------------------------------------

    env.push_back("SERVER_PROTOCOL=" + client.req.getHttpVersion());
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_SOFTWARE=Webserve");
    env.push_back("REQUEST_URI=" + client.req.getPath() + client.req.getQuery());
    env.push_back("SERVER_NAME="); // hostname needed later
    env.push_back("SERVER_PORT=" + to_string(client.port));
    env.push_back("REDIRECT_STATUS=200");

    // ------------- TODO: search about this two lines, if you need them ----------
	// env.push_back("SCRIPT_FILENAME=" + client.res.get_path());
    char *abs = realpath(client.res.get_path().c_str(), NULL);
    if (abs) {
        env.push_back("SCRIPT_FILENAME=" + std::string(abs));
        free(abs);
    }
    else
        env.push_back("SCRIPT_FILENAME=" + client.res.get_path());
	
    env.push_back("PATH_INFO=" + client.req.getPath());
    // ----------------------------------------------------------------------------

    std::map<std::string, std::string> headers = client.req.getHeaders();
    std::map<std::string, std::string>::iterator it = headers.begin();

    for (; it != headers.end(); it++)
    {
        std::string key = it->first;
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        std::replace(key.begin(), key.end(), '-', '_');

        if (key == "CONTENT_LENGTH" || key == "CONTENT_TYPE")
            env.push_back(key + "=" + it->second);
        else
            env.push_back("HTTP_" + key + "=" + it->second);
    }
}

void Cgi::buildEnv(Client &client)
{
    std::vector<std::string> env;
    size_t i;

    collectEnv(client, env);
    i = env.size();
    envp = new char *[i + 1];

    for (size_t j = 0; j < i; j++)
        envp[j] = strdup(env[j].c_str());
    envp[i] = NULL;
}

void Cgi::buildArg(Client &client)
{
    argv = new char *[3];

    argv[0] = strdup(interpreter.c_str());
    argv[1] = strdup(client.res.get_path().c_str());
    argv[2] = NULL;
}

void Cgi::setupCgi(Client &client)
{
    buildEnv(client);
    buildArg(client);
    state = CREAT_PIPES;
}

void Cgi::createPipes() {
    if (pipe(pipeIn) < 0 || pipe(pipeOut) < 0) {
        state = ERROR;
        return;
    }
    fcntl(pipeIn[1], F_SETFL, O_NONBLOCK);
    fcntl(pipeOut[0], F_SETFL, O_NONBLOCK);

    state = EXECUTING;
}

void Cgi::execution(Client &client)
{
    pid_t tmpid = fork();
    if (tmpid == -1)
    {
        std::cerr << "FORK FAILED" << std::endl;
        state = ERROR;
    }
    if (tmpid == 0)
        this->childProcess();
    else
    {
        pid = tmpid;
        this->parentProcess(client);
        state = CGI_READING;
    }
}

void Cgi::childProcess()
{
    std::string script_path = argv[1];
    std::string script_dir = script_path.substr(0, script_path.rfind('/'));
    std::string script_name = script_path.substr(script_path.rfind('/') + 1);

    if (!script_dir.empty())
        chdir(script_dir.c_str());

    free(argv[1]);
    argv[1] = strdup(script_name.c_str());

    if (dup2(pipeIn[0], STDIN_FILENO) == -1)
        exit(1);
    if (dup2(pipeOut[1], STDOUT_FILENO) == -1)
        exit(1);

    close(pipeIn[1]);
    close(pipeIn[0]);
    close(pipeOut[0]);
    close(pipeOut[1]);

    execve(argv[0], argv, envp);

    write(STDOUT_FILENO, "EXECVE FAILED: ", 16);
    // char *str = strerror(errno);
    // write(STDOUT_FILENO, str, strlen(str));
    exit(1);
}

void Cgi::parentProcess(Client &client)
{
	(void)client;
    close(pipeIn[0]);
    close(pipeOut[1]);
    // if (client.parse.body)
    //     write(
    //         pipeIn[1], client.req.getBody().c_str(), client.req.getBody().size());
    // close(pipeIn[1]);
    gettimeofday(&start, NULL);
}

// pipeOut[0] -> reading
// pipeIn[1] -> writing

// pipeIn[1] → CGI writes
// pipeOut[0] → your server reads → MUST be in epoll

// pipeOut[0] // parent reads (server)
// pipeOut[1] // child writes (CGI)

// pipeIn[0]  // child reads (CGI)
// pipeIn[1]  // parent writes (server)

// ------------------------------------------------------------------
// before performing reading i have to add the pipeOut to epoll???
// pipeOut[0] -> server reading
// pipeIn[1] -> server writing
// ------------------------------------------------------------------

// bool for the reading to called it once becouse of this

void Cgi::reading(int pipe_fd, unsigned int events, Client &client)
{
    const size_t CHUNK_SIZE = 8192;
    char buffer[CHUNK_SIZE];
    
    if (state != CGI_READING)
        return;
    
    // Read from CGI stdout pipe
    ssize_t n = read(pipe_fd, buffer, CHUNK_SIZE);
    std::cout << "[DEBUG CGI] read() returned " << n << " bytes" << std::endl;
    
    if (n > 0) {
        // Successfully read data
        std::cout << "[DEBUG CGI] Appending " << n << " bytes to response buffer (total now: " << (response.size() + n) << ")" << std::endl;
        response.append(buffer, n);
    }
    else if (n == 0) {
        // EOF from CGI
        std::cout << "[DEBUG CGI] EOF reached, CGI finished. Total output: " << response.size() << " bytes" << std::endl;
        state = CGI_WAITING;
    }
    else if (n == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            // Real read error
            std::cerr << "[DEBUG CGI] Read error: " << strerror(errno) << std::endl;
            state = ERROR;
        }
        // EAGAIN/EWOULDBLOCK: just return, will try again on next epoll event
    }
    
    // Always check process status and timeout
    checkResponseAndTime();
}

void Cgi::writing(int pipe_fd, unsigned int events, Client &client)
{
    if (state != CGI_READING && state != CGI_WAITING)
        return;
    
    const std::string &body = client.req.getBody();
    
    // If no body, just close the pipe
    if (body.empty()) {
        close(pipe_fd);
        return;
    }
    
    // Write remaining body to CGI stdin
    size_t remaining = body.size() - body_bytes_sent;
    if (remaining == 0) {
        // Already sent all
        close(pipe_fd);
        return;
    }
    
    ssize_t sent = write(pipe_fd, body.c_str() + body_bytes_sent, remaining);
    
    if (sent > 0) {
        body_bytes_sent += sent;
        
        // If all sent, close the write pipe
        if (body_bytes_sent >= (off_t)body.size()) {
            close(pipe_fd);
        }
    }
    else if (sent == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            // Real write error
            state = ERROR;
            close(pipe_fd);
        }
        // EAGAIN/EWOULDBLOCK: try again on next event
    }
}

void Cgi::checkResponseAndTime()
{
    if (response.size() > CGI_MAX_OUTPUT)
    {
        kill(pid, SIGTERM);
        state = ERROR;
        return;
    }

    pid_t waited = waitpid(pid, &status, WNOHANG);
    if (waited == pid)
    {
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            state = ERROR;
        else if (WIFSIGNALED(status))
        {
            std::cerr << "CGI process terminated by signal: " << WTERMSIG(status) << std::endl;
            state = ERROR;
        }
        else if (state == CGI_WAITING || state == CGI_READING)
            state = CGI_DONE;
    }
    else if (waited == 0)
    {
        gettimeofday(&current, NULL);
        if (current.tv_sec - start.tv_sec > CGI_TIMEOUT)
        {
			std::cout << BLUE << "WEEEEEE3333333333333333333333333" << RESET << std::endl;
            kill(pid, SIGTERM);
            state = ERROR;
        }
    }
}

void Cgi::handleCGI(Client &client)
{
    if (this->state == CHECKING)
        this->checkForCgi(client);

    if (this->state == SETUP_CGI) 
        this->setupCgi(client);

    if (this->state == CREAT_PIPES)
        this->createPipes();

    if (this->state == EXECUTING)
        this->execution(client);

    std::cout << "it's finish the cgi steps:" << this->state << std::endl;
}

int	Cgi::getPipeOutFd() const
{
	return (pipeOut[0]);
}

int Cgi::getPipeInFd() const {
	return pipeIn[1];
}
