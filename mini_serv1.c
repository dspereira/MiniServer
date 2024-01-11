#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>

#include <stdio.h>

void print_error(char *str);
int get_server_fd(char *port);
int accept_connection(int server_fd);
void handle_connection(int fd);

int main(int argc, char **argv)
{
	fd_set current_sockets, ready_sockets;
	int server_fd, client_fd;

	if (argc != 2)
		print_error("Wrong number of arguments\n");

	server_fd = get_server_fd(argv[1]);
	if (server_fd < 0)
		print_error("Fatal error\n");

	
	FD_ZERO(&current_sockets);
	FD_SET(server_fd, &current_sockets);

	while (1)
	{
		ready_sockets = current_sockets;
		if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
			print_error("Fatal error\n");
		
		for (int i=0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &ready_sockets))
			{
				if (i == server_fd)
				{
					client_fd = accept_connection(server_fd);
					if (client_fd < 0)
						print_error("Fatal error\n");
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

void print_error(char *str)
{
	int str_len;

	str_len = 0;
	while (str[str_len])
		str_len++;
	write(2, str, str_len);
	exit(1);
}

int get_server_fd(char *port)
{
	struct sockaddr_in	address;
	int server_fd;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		return (-1);
	
	bzero(&address, sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	address.sin_port = htons(atoi(port));

	if (bind(server_fd, (const struct sockaddr *) &address, sizeof(address)) < 0)
	{
		close(server_fd);
		return (-1);
	}
	
	if (listen(server_fd, 128) < 0)
	{
		close(server_fd);
		return (-1);
	}
	return (server_fd);
}

int accept_connection(int server_fd)
{
	struct sockaddr_in cli; 
	int len;
	int client_fd;

	len = sizeof(cli);
	client_fd = accept(server_fd, (struct sockaddr *)&cli, &len);

	return (client_fd);
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