#include "ft_webserv.hpp"

void	create_fd_set(int &max_d, int &ls, std::vector<Cl_socket> &store, fd_set &readfds)
{
	std::vector<Cl_socket>::iterator	it_store;
	int									fd_client;

	max_d = ls;

	FD_ZERO (&readfds); //очищаем множество
	FD_SET (ls, &readfds); //добавляем дескриптор в множество

	for(it_store = store.begin(); it_store != store.end(); ++it_store)
	{
		fd_client = Cl_socket(*it_store).fd;

		FD_SET(fd_client, &readfds); //добавляем дескриптор в множество
		if (fd_client > max_d)
			max_d = fd_client;
	}
}
