# MiniServer

This project is a simple C server designed to broadcast messages among multiple clients in localhost. It was created with the purpose of studying and gaining practical experience with the select() system call function in C and socket communications.

This program receives the 


## Installation / Usage

This project is designed to work on Linux and macOS. The server works on localhost (ip 127.0.0.1) and port defined by the user.

Clone repo:
```shell
git clone https://github.com/dspereira/MiniServer.git mini_server && cd mini_server
```

Build:
```shell
gcc mini_server.c -o mini_server
```

Usage example:

```shell
./mini_server 8080
```

## Examples

You can test the server by using the nc command (netcat) as a client for communication.
