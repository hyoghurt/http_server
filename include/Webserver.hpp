#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# define BUF_SIZE 1024

//#include <sys/time.h>
//#include <cstring>
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/un.h>
//#include <sstream>
//#include <fstream>
//#include <ctime>

# include <iostream>
# include <vector>
# include "Server.hpp"
# include "Client.hpp"

class	Webserver
{
	public:
		Webserver() {}
		Webserver(const Webserver& oth) { *this = oth; }
		~Webserver() {}

		Webserver&	operator= (const Webserver& oth)
		{
			this->server = oth.server;
			this->client = oth.client;
			return *this;
		}

		//debag_______________________________________________________
		void		makeServer(const std::string& ip, const int& port)
		{
			Server	serv;

			serv.setIpAddress(ip);
			serv.setPort(port);

			server.push_back(serv);
		}
		//____________________________________________________________

		int			createSocketListen(void)
		{
			std::vector<Server>::iterator	it;

			for (it = server.begin(); it != server.end(); ++it)
			{
				if (-1 == (*it).createSocketListen())
					return (-1);
			}
			return (1);
		}

		int			start(void)
		{
			fd_set		readfds;
			fd_set		writefds;
			int			max_d;
			int			num = 0;

			//мультиплексирование ввода-вывода select() poll() kqueue()
			while (1)
			{
				//создаем множество___________________________________________
				createFdSet(max_d, readfds, writefds);

				//создаем выборку файловых дескрипторов_______________________
				if (1 > select(max_d + 1, &readfds, &writefds, NULL, NULL))
				{
					print_error("select");
					if (num == 20)
						return (-1);
					++num;
					usleep(500000);
					continue ;
				}
				num = 0;

				//запрос от нового клиента____________________________________
				addNewClient(readfds);

				//данные клиента______________________________________________
				processingClient(readfds, writefds);
			}
			return (0);
		}
		
		void	createFdSet(int &max_d, fd_set &readfds, fd_set &writefds)
		{
			std::vector<Server>::iterator	it_server;
			std::vector<Client>::iterator	it_client;

			FD_ZERO(&readfds); //очищаем множество
			FD_ZERO(&writefds); //очищаем множество
			max_d = 0;

			//добавляем дескриптор в множество
			for (it_server = server.begin(); it_server != server.end(); ++it_server)
			{
				FD_SET((*it_server).getSocketListen(), &readfds);
				max_d = std::max((*it_server).getSocketListen(), max_d);
			}

			for (it_client = client.begin(); it_client != client.end(); ++it_client)
			{
				FD_SET((*it_client).getSocket(), &readfds);
				max_d = std::max((*it_client).getSocket(), max_d);
			}
		}

		void	addNewClient(fd_set &readfds)
		{
			std::vector<Server>::iterator	it_server;
			int								socket_listen;
			int								socket_client;
			struct sockaddr_in				addr_cl;
			socklen_t						addr_len;

			for (it_server = server.begin(); it_server != server.end(); ++it_server)
			{
				socket_listen = (*it_server).getSocketListen();
				if (FD_ISSET(socket_listen, &readfds))
				{
					//получаем сокет через который будет осуществлятся связь с клиентом
					socket_client = accept(socket_listen, (struct sockaddr*) &addr_cl, &addr_len);

					if (socket_client == -1)
						print_error("accept");
					else
					{
						fcntl(socket_client, F_SETFL, O_NONBLOCK);
						print_connect_info(socket_listen, socket_client, addr_cl);
						client.push_back(Client(socket_client));
					}
				}
			}
		}

		void	processingClient(fd_set &readfds, fd_set &writefds)
		{
			std::vector<Client>::iterator	it_client;
			int								socket_client;

			for (it_client = client.begin(); it_client != client.end(); ++it_client)
			{
				socket_client = (*it_client).getSocket();

				if (FD_ISSET(socket_client, &readfds))
					(*it_client).readSocket();
				if (FD_ISSET(socket_client, &writefds))
					;
					//read_cl_socket(*it_client);
			}
		}


	public:
		std::vector<Server>		server;
		std::vector<Client>		client;
};

/*
void			print_error(const std::string& str);
int				create_listen_socket(void);
std::string		ret_str_open(const std::string& file);
void			debag_pring_request(const int& fd_client, const std::string& str);
std::string		get_new_time(void);
void			print_connect_info(int fd_cl, struct sockaddr_in addr_cl);
void			read_cl_socket(Cl_socket cl_socket);
void			create_fd_set(int &max_d, int &ls, std::vector<Cl_socket> &store, fd_set &readfds);
void			add_fd_store(int &ls, std::vector<Cl_socket> &store);
*/

#endif
