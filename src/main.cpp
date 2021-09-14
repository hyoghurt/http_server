#include "ft_webserv.hpp"

void	create_fd_set(int &max_d, int &ls, std::vector<Cl_socket> &store, fd_set &readfds);
void	add_fd_store(int &ls, std::vector<Cl_socket> &store);
void	loop_store(std::vector<Cl_socket> &store, fd_set &readfds);

int	main()
{
	//слушающий сокет_________________________________________________
	int									ls;
	//хранилище клиентских сокетов____________________________________
	std::vector<Cl_socket>				store;
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

		//создаем выборку событий при изменений состояния файлового дескриптора
		if (select(max_d + 1, &readfds, NULL, NULL, NULL) < 1)
		{
			std::cout << RED << "Select < 1" << NO_C << '\n';
			print_error("accept");
			continue ;
		}

		//если получили запрос от нового клиента______________________
		if (FD_ISSET(ls, &readfds)) //fd слушающий сокет есть в выборке
			add_fd_store(ls, store);

		//пробегаем по хранилищу сокетов______________________________
		loop_store(store, readfds);

	}
    close (ls);
	return (0);
}

void	create_fd_set(int &max_d, int &ls, std::vector<Cl_socket> &store, fd_set &readfds)
{
	std::vector<Cl_socket>::iterator	it_store;
	int									fd_client;

	max_d = ls;

	FD_ZERO (&readfds); //очищаем множество
	FD_SET (ls, &readfds); //добавляем дескриптор в множество

	for(it_store = store.begin(); it_store != store.end(); ++it_store)
	{
		fd_client = Cl_socket(*it_store).get_fd();

		FD_SET(fd_client, &readfds); //добавляем дескриптор в множество
		if (fd_client > max_d)
			max_d = fd_client;
	}
}

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

void	loop_store(std::vector<Cl_socket> &store, fd_set &readfds)
{
	std::vector<Cl_socket>::iterator	it_store;

	char								buf[1024];
	int									result;
	int									fd_client;
    std::string							head;
	std::string							body;
    std::string							response;

	for(it_store = store.begin(); it_store != store.end(); ++it_store)
	{
		fd_client = Cl_socket(*it_store).get_fd();

		if (FD_ISSET(fd_client, &readfds)) //если дескриптор входит в множество
		{
			//Cl_socket(*it_store).set_time();

			//читаем данные с клиентского сокета в buf____________________________
			result = recv(fd_client, buf, sizeof(buf) - 1, 0);
			//if (result == -1)

			std::cout << get_new_time() << " fd:" << fd_client;
			std::cout << " get_byte:" << result << '\n';

			if (result > 0) //есть данные
			{
				buf[result] = '\0';

				Cl_socket(*it_store).set_request(Request(buf));
				debag_pring_request(fd_client, buf);

				//итоговый ответ___________________________________________________

				//отправляем ответ_________________________________________________
    			result = send(fd_client, response.c_str(), response.length() + 1, 0);

				//shutdown(Cl_socket, 2);
			}
		}
	}
}
