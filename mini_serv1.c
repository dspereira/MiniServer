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
int get_nl_index(char *str);
char *str_cut(char **buff, int idx);

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

int main(int argc, char **argv)
{
	t_client clients[65536];
	fd_set current_sockets, r_sockets, w_sockets;
	int server_fd, client_fd;
	int id = 0;
	int max_fd = 0;
	char *msg_send = 0;
	int nl_idx;
	char *msg_buff = 0;

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
		

		// aqui precisa de fazer update do msg_out.
		if (msg_buff)
		{
			printf("->%s", msg_buff);
			msg_buff = 0;
		}

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
					receive_msg(&clients[i], i);
					nl_idx = get_nl_index(clients[i].msg_in);
					while (nl_idx > -1)
					{
						msg_buff = str_join(msg_buff, str_cut(&(clients[i].msg_in), nl_idx));
						nl_idx = get_nl_index(clients[i].msg_in);
					}
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

void receive_msg(t_client *client, int fd)
{
	const int buff_size = 1000;
	char buff_read[buff_size + 1];
	int n_read;

	n_read = recv(fd, buff_read, buff_size, 0);
	if (n_read <= 0)
		exit(1); // neste caso tem simplesmente de fechar o cliente
	buff_read[n_read] = '\0';
	client->msg_in = str_join(client->msg_in, buff_read);
}

void prepare_messages(t_client *client, char msgs)
{

}

char *str_cut(char **buff, int idx)
{
	char *new_str;
	char *new_buff;
	int	 new_buff_size;

	if (idx < 0 || !(*buff))
		return (NULL);

	new_str = calloc(idx + 1, sizeof(char));
	for (int i=0; i <= idx; i++)
		new_str[i] = (*buff)[i];

	new_buff_size = strlen(*buff)-idx;
	if (new_buff_size < 2)
	{
		free(*buff);
		*buff = 0;
	}
	else
	{
		new_buff = calloc(new_buff_size, sizeof(char));
		for(int i=0; i < new_buff_size; i++)
			new_buff[i] = (*buff)[i];
		free(*buff);
		*buff = new_buff;
	}
	return (new_str);
}


//buff no final é mudificado removendo a menssagem
//cria novo array com a menssagem
/*char *get_msg(char **buff)
{
	char *msg = 0;
	int nl;

	nl = get_nl_index(*buff);
	if (nl < 0)
		return (NULL);
	
	if (nl == 0)
	{
		//msg = calloc(2, 
	}
	return (msg);
}
*/

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