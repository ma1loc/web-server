#include "cgi.hpp"
#include "../client.hpp"
#include "../utils/utils.hpp"
# include <fcntl.h>

// size_t Cgi::CGI_MAX_OUTPUT = 10000000;

size_t Cgi::CGI_MAX_OUTPUT = 200000000;

Cgi::Cgi()
{
	// exit(1);
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

// Cgi &Cgi::operator=(const Cgi &other)
// {
//     if (this != &other)
//     {
//         interpreter = other.interpreter;
//         extension = other.extension;
//     }
//     return *this;
// }

Cgi::~Cgi() {}

// Cgi::~Cgi()
// {
//     if (envp) {
//         for (size_t i = 0; envp[i]; i++)
//             free(envp[i]);
//         delete[] envp;
//         envp = NULL;
//     }
//     if (argv) {
//         for (size_t i = 0; argv[i]; i++)
//             free(argv[i]);
//         delete[] argv;
//         argv = NULL;
//     }
// }

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

void Cgi::checkForCgi(Client &client)
{
    // client.res.set_path("./www/cgi-bin/hello.py");
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
        if (exten == it->first)
        {
            interpreter = it->second;
            extension = it->first;
            state = SETUP_CGI;

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

        if (it->first == "CONTENT_LENGTH" || it->first == "CONTENT_TYPE")
            env.push_back(it->first + "=" + it->second);
        else
            env.push_back("HTTP_" + it->first + "=" + it->second);
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

// void Cgi::createPipes()
// {
//     if (pipe(pipeIn))
//     {
//         std::cerr << "PIPE FAILED" << std::endl;
//         state = ERROR;
//         return;
//     }
//     if (pipe(pipeOut))
//     {
//         std::cerr << "PIPE FAILED" << std::endl;
//         close(pipeIn[1]);
//         close(pipeIn[0]);
//         state = ERROR;
//         return;
//     }
//     state = EXECUTING;
// }

void Cgi::createPipes() {
    if (pipe(pipeIn) < 0 || pipe(pipeOut) < 0) {
        state = ERROR;
        return;
    }
    // YOU MUST ADD THESE LINES
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

// void Cgi::childProcess()
// {
// 	std::string script_path = argv[1];
//     std::string script_dir = script_path.substr(0, script_path.rfind('/'));
//     std::string script_name = script_path.substr(script_path.rfind('/') + 1); // ← ADD THIS

//     if (!script_dir.empty())
//         chdir(script_dir.c_str());
	
//     if (dup2(pipeIn[0], STDIN_FILENO) == -1)
//         exit(1);

//     if (dup2(pipeOut[1], STDOUT_FILENO) == -1)
//         exit(1);

//     close(pipeIn[1]);
//     close(pipeIn[0]);
//     close(pipeOut[0]);
//     close(pipeOut[1]);

//     execve(argv[0], argv, envp);

//     // Only runs if execve fails
//     write(STDOUT_FILENO, "EXECVE FAILED: ", 16);
//     char *str = strerror(errno);
//     write(STDOUT_FILENO, str, strlen(str));
//     exit(1);
// }

void Cgi::childProcess()
{
    std::string script_path = argv[1];
    std::string script_dir = script_path.substr(0, script_path.rfind('/'));
    std::string script_name = script_path.substr(script_path.rfind('/') + 1); // ← ADD THIS

    if (!script_dir.empty())
        chdir(script_dir.c_str());

    // Update argv[1] to just the filename after chdir
    free(argv[1]);
    argv[1] = strdup(script_name.c_str()); // ← use just "hello.py" not full path

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
    char *str = strerror(errno);
    write(STDOUT_FILENO, str, strlen(str));
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

// void Cgi::checkResponseAndTime()
// {
//     if (response.size() > CGI_MAX_OUTPUT)
//     {
//         kill(pid, SIGTERM);
//         // todo add counter for it the killing proccess
//         state = ERROR;
//         return;
//     }

//     pid_t wait = waitpid(pid, &status, WNOHANG);
//     if (wait == pid && state == CGI_WAITING)
//     {
//         if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
//             state = ERROR;
//         else
//             state = CGI_DONE;
//     }

// 	// --------------------------------
//     // if (wait == pid && state == CGI_WAITING)
//     // {
//     //     close(pipeOut[0]);
//     //     state = CGI_DONE;
//     // }
// 	// --------------------------------
    
//     else
//     {
//         gettimeofday(&current, NULL);
//         if (current.tv_sec - start.tv_sec > CGI_TIMEOUT)
//         {
//             kill(pid, SIGTERM);
//             // todo add counter for it the killing proccess
//             state = ERROR;
//         }
//     }
// }

// void Cgi::checkResponseAndTime()
// {
//     if (response.size() > CGI_MAX_OUTPUT)
//     {
// 		std::cerr << "CGI ERROR: output too large: " << response.size() << " > " << CGI_MAX_OUTPUT << std::endl;
// 		exit(1);
//         kill(pid, SIGTERM);
//         state = ERROR;
//         return;
//     }
//     pid_t wait = waitpid(pid, &status, WNOHANG);
//     if (wait == pid && state == CGI_WAITING)
//     {
//         if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
//         {
// 			std::cerr << "CGI ERROR: child exited with code " << WEXITSTATUS(status) << std::endl;
// 			exit(2);
//             state = ERROR;
//         }
//         else
//             state = CGI_DONE;
//     }
//     else
//     {
//         gettimeofday(&current, NULL);
//         if (current.tv_sec - start.tv_sec > CGI_TIMEOUT)
//         {
// 			std::cerr << "CGI ERROR: timeout after " << CGI_TIMEOUT << " seconds" << std::endl;
// 			exit(1);
//             kill(pid, SIGTERM);
//             state = ERROR;
//         }
//     }
// }

// void Cgi::reading()
// {
//     if (state == CGI_READING)
//     {
//         char buff[1024];

//         int n = read(pipeOut[0], buff, 1024);

//         if (n > 0)
//             response.append(buff, n);

// 		// --------------------------------
//         // else if (n == 0)
//         // {
//         //     state = CGI_WAITING;
//         //     close(pipeOut[0]);
//         // }
// 		// --------------------------------

// 		else if (n == 0)
//         {
// 			state = CGI_WAITING;
// 			close(pipeOut[0]);
// 			int waited = waitpid(pid, &status, WNOHANG);
// 			if (waited == pid)
//             {
//                 if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
//     		    	state = ERROR;  // in case of an error happend
//                 else
//                     state = CGI_DONE;
//             }
//         }
//         std::cout << "this->extension: " << this->extension << std::endl;
//         std::cout << "this->interpreter: " << this->interpreter << std::endl;
//         std::cout << "this->response: " << this->response << std::endl;
//     }
//     checkResponseAndTime();
// }

// void Cgi::reading() {
// 	const size_t buffer_size = 65536;
//     if (state == CGI_READING) {
//         char buff[buffer_size]; // Use a larger buffer (e.g., 64KB) for efficiency
//         int n = read(pipeOut[0], buff, sizeof(buff));
        
//         if (n > 0) {
//             response.append(buff, n);
//         }
//         else if (n == 0) {
//             // Only transition and close if the child is actually done
//             int waited = waitpid(pid, &status, WNOHANG);
//             if (waited == pid) {
//                 close(pipeOut[0]);
//                 if (WIFEXITED(status) && WEXITSTATUS(status) != 0) 
//                     state = ERROR;
//                 else 
//                     state = CGI_DONE;
//             } else if (waited == -1) {
//                 state = ERROR;
//                 close(pipeOut[0]);
//             }
//             // If waited == 0, child is still running. 
//             // Keep state as CGI_READING so epoll triggers again when child finishes/closes pipe.
//         }
//         else {
//             // Handle read error (e.g., EAGAIN)
//             if (errno != EAGAIN && errno != EWOULDBLOCK) {
//                 state = ERROR;
//                 close(pipeOut[0]);
//             }
//         }
//     }
//     checkResponseAndTime();
// }

void Cgi::reading() {
    const size_t buffer_size = 65536;
    if (state == CGI_READING) {
        char buff[buffer_size];
        int n = read(pipeOut[0], buff, sizeof(buff));
        if (n > 0) {
            response.append(buff, n);
        }
        else if (n == 0) {
            // pipe EOF — child closed stdout
            state = CGI_WAITING;
            close(pipeOut[0]);
            // checkResponseAndTime() below will waitpid
        }
        else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                state = ERROR;
                close(pipeOut[0]);
            }
        }
    }
    checkResponseAndTime();
}

void Cgi::checkResponseAndTime()
{
    if (response.size() > CGI_MAX_OUTPUT)
    {
        kill(pid, SIGTERM);
        state = ERROR;
        return;
    }
    // ← key fix: removed state == CGI_WAITING check
    // reap child regardless of current state
    pid_t waited = waitpid(pid, &status, WNOHANG);
    if (waited == pid)
    {
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            state = ERROR;
        else if (state == CGI_WAITING || state == CGI_READING)
            state = CGI_DONE;
    }
    else if (waited == 0)
    {
        // child still running — check timeout
        gettimeofday(&current, NULL);
        if (current.tv_sec - start.tv_sec > CGI_TIMEOUT)
        {
            kill(pid, SIGTERM);
            state = ERROR;
        }
    }
    // waited == -1 means already reaped — do nothing
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
    if (this->state == CGI_READING ||
        this->state == CGI_WAITING)
        this->reading();
}

// ----------- new --------------
int	Cgi::getPipeFd() const
{
	return (pipeOut[0]);
}

int Cgi::getPipeInFd() const {
	return pipeIn[1];
}