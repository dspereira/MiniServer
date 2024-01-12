#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <strings.h>


#include <stdio.h>

typedef struct s_client
{
	int id;
	int fd;
	char *msg_in;
	char *msg_out;
} t_client;

void print_error(char *str);
int get_server_fd(char *port);
int accept_connection(int server_fd);
void handle_connection(int fd);
void update_max_fd(int *max_fd, int fd);
void receive_msg(t_client *client, int fd);
int str_len(char *str);

int main(int argc, char **argv)
{
	t_client clients[65536];
	fd_set current_sockets, r_sockets, w_sockets;
	int server_fd, client_fd;
	int id = 0;
	int max_fd = 0;
	char *msg_send = 0;

	if (argc != 2)
		print_error("Wrong number of arguments\n");

	server_fd = get_server_fd(argv[1]);
	if (server_fd < 0)
		print_error("Fatal error\n");

	update_max_fd(&max_fd, server_fd);
	bzero(clients, sizeof(clients));
	FD_ZERO(&current_sockets);
	FD_SET(server_fd, &current_sockets);

	while (1)
	{
		r_sockets =  w_sockets = current_sockets;
		if (select(max_fd+1, &r_sockets, &w_sockets, NULL, NULL) < 0)
			print_error("Fatal error\n");
		
		for (int i=0; i < max_fd+1; i++)
		{
			if (FD_ISSET(i, &r_sockets))
			{
				if (i == server_fd)
				{
					client_fd = accept_connection(server_fd);
					if (client_fd < 0)
						print_error("Fatal error\n");
					update_max_fd(&max_fd, client_fd);
					FD_SET(client_fd, &current_sockets);
				}
				else
				{
					//handle_connection(i);
					//receive_msg(&clients[i], i);
					printf("msg: %s\n", clients[i].msg_in);
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
	char buffer[2];
	int n_read;

	n_read = read(fd, buffer, 1);
	buffer[n_read] = '\0';

	printf("%s", buffer);

	write(fd, "resposta\n", 9);
}

void update_max_fd(int *max_fd, int fd)
{
	if (fd > *max_fd)
		*max_fd = fd;
}

/*
// verificar se a ligacao fecha
void receive_msg(t_client *client, int fd)
{
	const int buff_size = 2;
	char buff_read[buff_size];
	char *buff; 
	int read_bytes;
	int total_size;

	read_bytes = recv(fd, buff_read, buff_size-1, 0);
	buff_read[read_bytes] = '\0';
	total_size = str_len(client->msg_in) + read_bytes + 1;

	printf("client->msg_in: %s\n", client->msg_in);
	printf("total size: %i\n", total_size);

	buff = malloc(total_size);

	if (client->msg_in)
	{
		strcat(buff, client->msg_in);
		free(client->msg_in);
	}
	strcat(buff, buff_read);
	buff[total_size] = '\0';
	client->msg_in = buff;
}

int str_len(char *str)
{
	if (!str)
		return (0);
	return (strlen(str));
}
*/

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}
