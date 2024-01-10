#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

#include <stdio.h>

// nc -nv 127.0.0.1 8080

void print_str(int fd, char *str);
int get_server_fd(char *port);
int accept_connection(int server_fd);
void handle_connection(int fd);

int main(int argc, char **argv)
{
	int server_fd;
	int client_fd;
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

	fd_set current_sockets, ready_sockets;
	FD_ZERO(&current_sockets);
	FD_SET(server_fd, &current_sockets);

	while (1)
	{
		ready_sockets = current_sockets;
		
		if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
		{
			print_str(2, "Select error\n");
			return (-1);
		}

		for (int i=0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &ready_sockets))
			{
				if (i == server_fd)
				{
					client_fd = accept_connection(server_fd);
					FD_SET(client_fd, &current_sockets);
				}
				else
				{
					handle_connection(i);
				}
			}
		}
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

int accept_connection(int server_fd)
{
	int new_connect_fd;

	struct sockaddr_in	addr;
	socklen_t			addrlen;

	addrlen = (socklen_t)sizeof(addr);
	new_connect_fd = accept(server_fd, (struct sockaddr *) &addr, &addrlen);
	return (new_connect_fd);
}

void handle_connection(int fd)
{
	char buffer[1000];
	int n_read;

	n_read = read(fd, buffer, 999);
	buffer[n_read] = '\0';

	printf("%s", buffer);

	write(fd, "resposta\n", 9);
}