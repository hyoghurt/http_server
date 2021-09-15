#include "ft_webserv.hpp"

void	add_fd_store(int &ls, std::vector<Cl_socket> &store)
{
	int									fd_client;
	struct sockaddr_in					addr_cl;
	socklen_t							addr_len;

	//получаем сокет через который будет осуществлятся связь с клиентом
	fd_client = accept(ls, (struct sockaddr*) &addr_cl, &addr_len);

	if (fd_client == -1)
		print_error("accept");
	else
	{
		print_connect_info(fd_client, addr_cl);
		store.push_back(Cl_socket(fd_client)); //save socket client
	}
}
