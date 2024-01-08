#include <unistd.h>

void print_str(int fd, char *str);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        print_str(2, "Wrong number of arguments\n");
        return (1);
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
