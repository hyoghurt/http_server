#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include "ft_webserv.hpp"

int		create_listen_socket(void)
{
	//создание сокета ___________________________________________________
	//int socket(int domain, int type, int protocol)
	//IF_INET - IPv4 протоколы Интернет
	//SOCK_STREAM - обеспечивает создание двусторонних надежных и последовательных
	//				потоков байтов , поддерживающих соединения
	int	ls = socket(AF_INET, SOCK_STREAM, 0);
	if (ls == -1)
	{
		print_error("socket");
		return (0);
	}

	//перевод сокета в неблокирующий режим____________________________
	//нужн перевести слушащий и клиентский сокет в данный режим_______
	int	flags = fcntl(ls, F_GETFL);
	fcntl(ls, F_SETFL, flags | O_NONBLOCK);

	//установили флаг, для предотвращения залипаниz TCP порта__________
	int	opt = 1;
	if (-1 == setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		print_error("setsockopt");
		return (0);
	}

	//сопостовление созданному сокету конкретного адреса________________
	//int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
	struct sockaddr_in  addr;

	addr.sin_family = AF_INET; //семейство адресов (IF_INET - IPv4 протоколы Интернет)
	addr.sin_port = htons(8000); //номер портa (8000)
	addr.sin_addr.s_addr = INADDR_ANY; //IP адрес (INADDR_ANY это 0.0.0.0 (все адреса))

	if (-1 == bind(ls, (struct sockaddr*) &addr, sizeof(addr)))
	{
		print_error("bind");
		close(ls);
		return (0);
	}

	//переводим сокет в слушаюший режим_________________________________
	if (-1 == listen(ls, 5))
	{
		print_error("listen");
		close(ls);
		return (0);
	}
	return (ls);
}
