#include "ft_webserv.hpp"

int	main()
{
    //проверка
	//слушающий сокет_________________________________________________
	int									ls;
	//хранилище клиентских сокетов____________________________________
	std::vector<Cl_socket>				store;
	std::vector<Cl_socket>::iterator	it_store;
	//множество дескрипторов__________________________________________
	fd_set								readfds;
	fd_set								writefds;
	//переменная для select___________________________________________
	int									max_d;

	//читаем конфиг файл______________________________________________

	//создаем слушающий сокет_________________________________________
	ls = create_listen_socket();

	//мультиплексирование ввода-вывода select() poll() kqueue()
	while(1)
	{
		//создаем множество___________________________________________
		create_fd_set(max_d, ls, store, readfds);

		//создаем выборку файловых дескрипторов_______________________
		if (select(max_d + 1, &readfds, NULL, NULL, NULL) < 1)
		{
			std::cout << RED << "Select < 1" << NO_C << '\n';
			print_error("select");
			continue ;
		}

		//получили запрос от нового клиента___________________________
		if (FD_ISSET(ls, &readfds))
			add_fd_store(ls, store);

		//пробегаем по хранилищу сокетов______________________________
		for(it_store = store.begin(); it_store != store.end(); ++it_store)
		{
			//имеются данные на чтение от клиента_____________________
			if (FD_ISSET(Cl_socket(*it_store).fd, &readfds))
				read_cl_socket(Cl_socket(*it_store));
			/*
			if (FD_ISSET(cl_socket.fd, &writefds))
    			//result = send(fd_client, response.c_str(), response.length() + 1, 0);
			*/
		}
	}
    close (ls);
	return (0);
}
