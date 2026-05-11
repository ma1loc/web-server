*This project has been created as part of the 1337/42 curriculum by [@me](https://github.com/ma1loc), [@BusCops](https://github.com/BusCops) and [@YbencheL](https://github.com/YbencheL).*

## Description

This project focuses on building our own HTTP web server from scratch. The goal is to understand how web servers work internally by implementing their core components and handling real HTTP communication.

To better understand the scope of this project, it is important to first define what an HTTP web server is.

HTTP (HyperText Transfer Protocol) is a protocol built on top of TCP/IP. It allows communication between two machines through a request/response model.

HTTP is a stateless protocol, meaning that each request is independent and the connection does not retain information between requests. This is why mechanisms like *cookies* are used—to simulate stateful behavior on top of a stateless protocol.

In HTTP, client interactions are performed using methods such as GET, POST, DELETE, etc. Each request sent by the client is processed by the server, which then returns a response.

As for the HTTP response, it is mainly composed of two parts:
- A status code (indicating the result of the request)
- A body (the data returned to the client)

## How my HTTP Server works:

<img width="1716" height="820" alt="Screenshot From 2026-05-02 11-15-45" src="https://github.com/user-attachments/assets/6ce258ab-8447-42c2-9395-9cf73a89d6c3" />

<img width="995" height="952" alt="Screenshot From 2026-05-02 11-16-13" src="https://github.com/user-attachments/assets/0e28919a-3523-4f84-8b14-76f2eea6cb92" />

<img width="1032" height="480" alt="Screenshot From 2026-05-02 11-17-55" src="https://github.com/user-attachments/assets/c3b78614-f4e7-40d2-a357-2d55e0410897" />

<img width="1120" height="586" alt="Screenshot From 2026-05-02 11-18-08" src="https://github.com/user-attachments/assets/52b13926-61cd-4c24-ade9-4f2bb166529f" />

<img width="1070" height="905" alt="Screenshot From 2026-05-02 11-17-41" src="https://github.com/user-attachments/assets/49683b4f-9725-4d8e-9dde-6ea4391b981f" />

<img width="1709" height="461" alt="Screenshot From 2026-05-02 11-18-35" src="https://github.com/user-attachments/assets/bfc0776b-316c-4e65-b700-8ae060d14828" />

<img width="1788" height="538" alt="Screenshot From 2026-05-02 11-18-49" src="https://github.com/user-attachments/assets/0b70c12e-3694-4d62-ab72-4b11fe0ece59" />


<img width="1641" height="952" alt="Screenshot From 2026-05-02 11-16-55" src="https://github.com/user-attachments/assets/75ee9040-54d2-42bf-aa0a-0fa7402437c8" />


<img width="1070" height="540" alt="Screenshot From 2026-05-02 11-17-18" src="https://github.com/user-attachments/assets/fe15f8be-ee39-4fa8-a3f6-0047d0234411" />

FOR more details, check my Medium [here](https://medium.com/p/d78223b4573a/edit)



## What [@me](https://github.com/ma1loc) built:
- Multiplier management  
- Validation of config file data after parsing  
- Request validation  
- Full HTTP response construction  
- Request routing  
- Methods (GET, POST, DELETE)  
- Dynamic HTML generation based on error status codes  
- Autoindex navigation page  
- Testing

## What [@BusCops](https://github.com/BusCops) built:
- Parsing requests coming from the client browser  
- CGI I/O handling  

## What [@YbencheL](https://github.com/YbencheL) built:
- Config file parsing  
- Cookies and session management  

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

---

## 🧪 Testing

### Local Stress Test

> `./stress-test`

| #  | Method | URL | Notes |
|----|--------|-----|-------|
| 1  | `GET`  | `http://localhost:8080/` | |
| 2  | `POST` | `http://localhost:8080/` | Body size: `0` |
| 3  | `HEAD` | `http://localhost:8080/` | |
| 4  | `GET`  | `http://localhost:8080/directory` | |
| 5  | `GET`  | `http://localhost:8080/directory/youpi.bad_extension` | |
| 6  | `GET`  | `http://localhost:8080/directory/youpi.bla` | |
| 7  | `GET`  | `http://localhost:8080/directory/oulalala` | Expected `404` |
| 8  | `GET`  | `http://localhost:8080/directory/nop` | |
| 9  | `GET`  | `http://localhost:8080/directory/nop/` | |
| 10 | `GET`  | `http://localhost:8080/directory/nop/other.pouic` | |
| 11 | `GET`  | `http://localhost:8080/directory/nop/other.pouac` | Expected `404` |
| 12 | `GET`  | `http://localhost:8080/directory/Yeah` | Expected `404` |
| 13 | `GET`  | `http://localhost:8080/directory/Yeah/not_happy.bad_extension` | |
| 14 | `POST` | `http://localhost:8080/directory/youpi.bla` | Body size: `100,000,000` |
| 15 | `POST` | `http://localhost:8080/directory/youpla.bla` | Body size: `100,000,000` |
| 16 | `POST` | `http://localhost:8080/directory/youpi.bla` | Body size: `100,000` · Special headers |
| 17 | `POST` | `http://localhost:8080/post_body` | Body size: `0` |
| 18 | `POST` | `http://localhost:8080/post_body` | Body size: `100` |
| 19 | `POST` | `http://localhost:8080/post_body` | Body size: `200` |
| 20 | `POST` | `http://localhost:8080/post_body` | Body size: `101` |
| 21 | `GET`  | `/` | `5` workers × `15` requests |
| 22 | `GET`  | `/` | `20` workers × `5,000` requests |
| 23 | `GET`  | `/directory/nop` | `128` workers × `50` requests |
| 24 | `POST` | `/directory/youpi.bla` | `20` workers × `5` requests · Body size: `100,000,000` |

✅ **All tests passed!**

---

### Siege Benchmark

> `siege -c 255 -t 1s http://localhost:8080`

| Metric | Result |
|--------|--------|
| **Availability** | `100.00%` |
| **Transactions** | `7,332 hits` |
| **Elapsed Time** | `1.97 s` |
| **Data Transferred** | `183.52 MB` |
| **Response Time** | `64.44 ms` |
| **Transaction Rate** | `3,721.83 trans/sec` |
| **Throughput** | `93.16 MB/sec` |
| **Concurrency** | `239.85` |
| **Successful Transactions** | `7,332` |
| **Failed Transactions** | `0` |
| **Longest Transaction** | `320.00 ms` |
| **Shortest Transaction** | `0.00 ms` |

> Tested with **255 concurrent users** over **1 second** — zero failures recorded.