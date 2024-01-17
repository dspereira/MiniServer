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
void update_max_fd(int *max_fd, int fd);
int receive_msg(t_client *client);
void send_msg(t_client *client);
char *str_join(char *buf, char *add);
char *str_cut(char **buff, int idx);
int get_nl_index(char *str);
int add_new_client(t_client *clients, int server_fd);
void remove_client(t_client *client);
void update_client_msg(t_client *clients, char *msg, int max_fd, int fd);
void update_server_msg(t_client *clients, int max_fd, int fd, int state);
void write_client_id_info(t_client *clients);
void	*oom_guard(void *p);


int main(int argc, char **argv)
{
	t_client clients[65536];
	fd_set current_sockets, r_sockets, w_sockets;
	int server_fd, client_fd;
	int max_fd = 0;
	int nl_idx;

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

		for (int i=0; i <= max_fd; i++)
		{
			if (FD_ISSET(i, &r_sockets))
			{
				if (i == server_fd)
				{
					client_fd = add_new_client(clients, server_fd);
					if (client_fd < 0)
						print_error("Fatal error\n");
					update_max_fd(&max_fd, client_fd);
					FD_SET(client_fd, &current_sockets);
					update_server_msg(clients, max_fd, client_fd, 0);
				}
				else
				{
					if (receive_msg(&clients[i]) < 0)
					{
						update_server_msg(clients, max_fd, i, 1);
						remove_client(&clients[i]);
						FD_CLR(i, &current_sockets);
					}
                    else
                    {
                        nl_idx = get_nl_index(clients[i].msg_in);
                        while (nl_idx > -1)
                        {
                            update_client_msg(clients, str_cut(&(clients[i].msg_in), nl_idx), max_fd, i);
                            nl_idx = get_nl_index(clients[i].msg_in);
                        }
                    }
				}
			}
			if (FD_ISSET(i, &w_sockets))
				send_msg(&clients[i]);
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

void update_max_fd(int *max_fd, int fd)
{
	if (fd > *max_fd)
		*max_fd = fd;
}

int receive_msg(t_client *client)
{
	const int buff_size = 100000;
	char buff_read[buff_size + 1];
	int n_read;

	n_read = recv(client->fd, buff_read, buff_size, 0);
	if (n_read <= 0)
		return (-1);
	buff_read[n_read] = '\0';
	client->msg_in = str_join(client->msg_in, buff_read);
	return (n_read);
}

void send_msg(t_client *client)
{
	int n_send;
	int msg_len;

	if (!client->msg_out)
		return ;
	msg_len = strlen(client->msg_out);
	n_send = send(client->fd, client->msg_out, msg_len, 0);

	if (msg_len == n_send)
	{
		free(client->msg_out);
		client->msg_out = 0;
		return ;
	}
	if (n_send > 0)
		client->msg_out = str_cut(&(client->msg_out), n_send);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = oom_guard(malloc(sizeof(*newbuf) * (len + strlen(add) + 1)));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

char *str_cut(char **buff, int idx)
{
	char	*new_str;
	char	*new_buff;
	int		buff_len;
	int		idx2;

	if (idx < 0 || !(*buff))
		return (NULL);
	if (idx == 0)
		new_str = oom_guard(calloc(2, sizeof(char)));
	else
		new_str = oom_guard(calloc(idx + 2, sizeof(char)));
	for (int i=0; i <= idx; i++)
		new_str[i] = (*buff)[i];
	buff_len = strlen(*buff);
	if(buff_len == idx + 1)
	{
		free(*buff);
		*buff = NULL;
		return (new_str);
	}
	new_buff = oom_guard(calloc((strlen(*buff) - idx) + 1, sizeof(char)));
	idx++;
	idx2 = 0;
	while ((*buff)[idx])
	{
		new_buff[idx2] = (*buff)[idx];
		idx++;
		idx2++;
	}
	free(*buff);
	*buff = new_buff;
	return (new_str);
}

int get_nl_index(char *str)
{
	int i = 0;

	if (!str)
		return (-1);
	while (str[i])
	{
		if (str[i] == '\n')
        	return (i);
		i++;
	}
	return (-1);
}

int add_new_client(t_client *clients, int server_fd)
{
	static int id = 0;
	struct sockaddr_in cli; 
	socklen_t len;
	int fd;

	len = (socklen_t)sizeof(cli);
	fd = accept(server_fd, (struct sockaddr *)&cli, &len);
	if (fd < 0)
		return (-1);
	clients[fd].fd = fd;
	clients[fd].id = id;
	clients[fd].msg_in = 0;
	clients[fd].msg_out = 0;
	id++;
	return (fd);
}

void remove_client(t_client *client)
{
	close(client->fd);
	client->fd = 0;
	client->id = 0;
	if (client->msg_in)
		free(client->msg_in);
	if (client->msg_out)
		free(client->msg_out);
	client->msg_in = 0;
	client->msg_out = 0;
}

void update_client_msg(t_client *clients, char *msg, int max_fd, int fd)
{
	char prefix[50];

	if (!msg)
		return ;
    bzero(prefix, 50);
	sprintf(prefix, "client %d: ", clients[fd].id);
	for (int i=0; i <= max_fd; i++)
	{
		if (clients[i].fd > 0 && clients[i].fd != fd)
		{
			clients[i].msg_out = str_join(clients[i].msg_out, prefix);
			clients[i].msg_out = str_join(clients[i].msg_out, msg);
		}
	}
}

void update_server_msg(t_client *clients, int max_fd, int fd, int state)
{
	char msg[50];

    bzero(msg, 50);
	if (state == 0)
		sprintf(msg, "server: client %d just arrived\n", clients[fd].id);
	else
		sprintf(msg, "server: client %d just left\n", clients[fd].id);
	write_client_id_info(&clients[fd]);
	for (int i=0; i <= max_fd; i++)
	{
		if (clients[i].fd > 0 && clients[i].fd != fd)
			clients[i].msg_out = str_join(clients[i].msg_out, msg);
	}	
}

void write_client_id_info(t_client *client)
{
	char msg[200];

	bzero(msg, 200);
	sprintf(msg, "----------------\n|   CLIENT %d   |\n----------------\n", client->id);
	client->msg_out = str_join(client->msg_out, msg);
}

void	*oom_guard(void *p)
{
	if (!p)
	{
		write(2, "Fatal error\n", strlen("Fatal error\n"));
		exit(1);
	}
	return (p);
}
