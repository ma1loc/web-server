# WEBSERV

*This project has been created as part of the 42 curriculum by [abenzaho], [yanflous], and [ybenchel].*

## Description

This project focuses on building our own HTTP web server from scratch. The goal is to understand how web servers work internally by implementing their core components and handling real HTTP communication.

To better understand the scope of this project, it is important to first define what an HTTP web server is.

HTTP (HyperText Transfer Protocol) is a protocol built on top of TCP/IP. It allows communication between two machines through a request/response model.

HTTP is a stateless protocol, meaning that each request is independent and the connection does not retain information between requests. This is why mechanisms like *cookies* are used—to simulate stateful behavior on top of a stateless protocol.

In HTTP, client interactions are performed using methods such as GET, POST, DELETE, etc. Each request sent by the client is processed by the server, which then returns a response.

As for HTTP response it is mainly composed of two parts:
- A status code (indicating the result of the request)
- A body (the data returned to the client)

## Instructions

To run the web server, first compile the project using `make`. This will build the executable `webserv`, which requires a configuration file as an argument.

You can find example configuration files inside the `config_files` directory.

### Available Make commands

- `make`  
  Compiles all source files and generates the `webserv` executable.

- `make clean`  
  Removes all object files.

- `make fclean`  
  Removes all object files as well as the executable.

- `make re`  
  Recompiles the entire project (equivalent to running `make fclean` followed by `make`).

### Running the server

run this on the terminal: ./webserv <configuration_file>

### Testing for leaks

run this on the terminal: valgrind --track-origins=yes ./webserv <configuration_file>

## Resources

For this project, we relied on a combination of personal research and well-known documentation.

One of our teammates, yanflous, created a helpful resource that you can find here:  
<https://medium.com/@ma1loc/web-d78223b4573a>

Since this project revolves around HTTP and web servers, there are many high-quality references available. Here are some of the most useful ones:

- Nginx documentation (configuration and behavior):  <https://nginx.org/en/docs/>

### AI Usage in the Project

AI tools were mainly used as a sort of an interactive documentation platfrom. They helped us better understand specific concepts, such as how certain components work and how they behave in different scenarios.

Additionally, AI was used for:
- Providing ideas for testing different parts of the project
- Assisting with grammar and wording improvements in this README

AI used as a support tool to enhance understanding and improve documentation quality.

# Webserv Evaluation - Test Results

This document contains the output of the official `webserv` tester.

## Environment Setup
- **OS**: Linux (Debian Trixie)
- **Port**: 8080 / 9090
- **Tester**: `./testers/tester`

## Tester Output

```text
Welcome in this little webserver tester.
Passing the test here is the minimum before going to an evaluation.

THIS TEST IS NOT MEANT TO BE THE ONLY TEST IN THE EVALUATION!!!

Before starting please follow the next few steps (files content can be anything):
- Download the cgi_test executable on the host
- Create a directory YoupiBanane with:
    - A file name youpi.bad_extension
    - A file name youpi.bla
    - A sub directory called nop
        - A file name youpi.bad_extension in nop
        - A file name other.pouic in nop
    - A sub directory called Yeah
        - A file name not_happy.bad_extension in Yeah

Setup the configuration file as follow:
- / must answer to GET request ONLY
- Any file with .bla as extension must answer to POST request by calling the cgi_test executable
- /post_body must answer anything to POST request with a maxBody of 100
- /directory/ must answer to GET request and the root of it would be the repository YoupiBanane and if no file are requested, it should search for youpi.bad_extension files

Test GET http://localhost:8080/
content returned: [HTML Directory Listing]

Test POST http://localhost:8080/ with a size of 0
Test HEAD http://localhost:8080/

Test GET http://localhost:8080/directory
content returned: youpi bad extension file

Test GET http://localhost:8080/directory/youpi.bad_extension
content returned: youpi bad extension file

Test GET http://localhost:8080/directory/youpi.bla
content returned: 

Test GET Expected 404 on http://localhost:8080/directory/oulalala
content returned: [404 Error Page]

Test GET http://localhost:8080/directory/nop
content returned: youpi bad extension in nop

Test GET http://localhost:8080/directory/nop/
content returned: youpi bad extension in nop

Test GET http://localhost:8080/directory/nop/other.pouic
content returned: other pouic file

Test GET Expected 404 on http://localhost:8080/directory/nop/other.pouac
content returned: [404 Error Page]

Test GET Expected 404 on http://localhost:8080/directory/Yeah
content returned: [404 Error Page]

Test GET http://localhost:8080/directory/Yeah/not_happy.bad_extension
content returned: not happy bad extension

Test POST http://localhost:8080/directory/youpi.bla with a size of 100000000
Test POST http://localhost:8080/directory/youpla.bla with a size of 100000000
Test POST http://localhost:8080/directory/youpi.bla with a size of 100000 with special headers

Test POST http://localhost:8080/post_body with a size of 0
Test POST http://localhost:8080/post_body with a size of 100
Test POST http://localhost:8080/post_body with a size of 200
Test POST http://localhost:8080/post_body with a size of 101

Test multiple workers(5) doing multiple times(15): GET on /
Test multiple workers(20) doing multiple times(5000): GET on /
Test multiple workers(128) doing multiple times(50): GET on /directory/nop
Test multiple workers(20) doing multiple times(5): Post on /directory/youpi.bla with size 100000000

********************************************************************************
GG, So far so good! Run your own tests now! :D
********************************************************************************

<img width="1280" height="699" alt="580473615-bc72d75e-19db-4c20-8baa-75aa78b756f4" src="https://github.com/user-attachments/assets/94de6e22-3787-4b05-8067-82b0c5f5be1a" />

