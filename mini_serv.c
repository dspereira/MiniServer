#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdio.h>

void print_str(int fd, char *str);
int get_server_fd(char *port);

int main(int argc, char **argv)
{
    int server_fd;
    if (argc != 2)
    {
        print_str(2, "Wrong number of arguments\n");
        return (1);
    }
    server_fd = get_server_fd(argv[1]);
    if (server_fd < 0)
    {
        print_str(2, "Fatal error\n");
        return (-1);
    }
    return (0);
}

void print_str(int fd, char *str)
{
    int str_size;

    str_size = 0;
    while (str[str_size])
        str_size++;
    write(fd, str, str_size);
}

int get_server_fd(char *port)
{
    struct addrinfo		hints, *result;
    struct sockaddr_in	address;
    int server_fd;

    memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;			// IPv4
	hints.ai_socktype = SOCK_STREAM;	// TCP/IP protocol

    if (getaddrinfo("127.0.0.1", port, &hints, &result) != 0)
        return (-1);

    address = *((struct sockaddr_in *)(result->ai_addr));
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int enable = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
	{
		close(server_fd);
		freeaddrinfo(result);
		return (-1);
	}
	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) 
	{
		close(server_fd);
		freeaddrinfo(result);
    	return (-1);
	}
    if (listen(server_fd, 128) < 0)
    {
		close(server_fd);
		freeaddrinfo(result);
        return (-1);
    }
	freeaddrinfo(result);
    // maybe is necessary save in a variable on main (struct sockaddr_in address)
    return (server_fd);
}