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
		/*
		Webserver(const Webserver& oth) { *this = oth; }
		*/
		~Webserver()
		{
			std::vector<int>::iterator	it;

			for (it = listenSocket.begin(); it != listenSocket.end(); ++it)
			{
				std::cout << "Webserver: Close socket listen: " << *it << std::endl;
				close (*it);
			}
		}

		/*
		Webserver&	operator= (const Webserver& oth)
		{
			this->server = oth.server;
			this->client = oth.client;
			this->listenSocket = oth.listenSocket;
			return *this;
		}
		*/

		//debag_______________________________________________________
		void		makeServer(Server& serv)
		{
			server.push_back(serv);
		}
		//____________________________________________________________


		int			createSocketListen(void)
		{
			std::vector<Server>::iterator	it;
			std::vector<int>::iterator		it_int;
			std::string						ip;
			int								port;
			int								socketListen;

			for (it = server.begin(); it != server.end(); ++it)
			{
				ip = (*it).getIpAddress();
				port = (*it).getPort();

				if (-1 != checkIpAddressAndPort(ip, port, it))
				{
					socketListen = createSocketListen(ip, port);
					if (-1 == socketListen)
						return (-1);
					listenSocket.push_back(socketListen);
				}
			}
			return (0);
		}

		int			checkIpAddressAndPort(const std::string& ip, const int& port, 
				std::vector<Server>::iterator it_end)
		{
			std::vector<Server>::iterator	it;

			for (it = server.begin(); it != it_end; ++it)
			{
				if ((*it).getIpAddress() == ip and (*it).getPort() == port)
					return (-1);
			}
			return (0);
		}

		int			createSocketListen(const std::string& ip, const int& port)
		{
			int	socketListen;

			//создание сокета ___________________________________________________
			socketListen = socket(AF_INET, SOCK_STREAM, 0);
			if (socketListen == -1)
			{
				print_error("socket");
				return (-1);
			}

			//перевод сокета в неблокирующий режим____________________________
			int	flags = fcntl(socketListen, F_GETFL);
			if (-1 == fcntl(socketListen, F_SETFL, flags | O_NONBLOCK))
			{
				print_error("fcntl");
				return (-1);
			}

			//установили флаг, для предотвращения залипаниz TCP порта__________
			int	opt = 1;
			if (-1 == setsockopt(socketListen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
			{
				print_error("setsockopt");
				return (-1);
			}

			//сопостовление созданному сокету конкретного адреса________________
			struct sockaddr_in  addr;

			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = inet_addr(ip.c_str());
			if (-1 == bind(socketListen, (struct sockaddr*) &addr, sizeof(addr)))
			{
				print_error("bind");
				return (-1);
			}

			//переводим сокет в слушаюший режим_________________________________
			if (-1 == listen(socketListen, 5))
			{
				print_error("listen");
				return (-1);
			}

			//печать на консоль информацию______________________________________
			std::cout << PINK << "________START SERVER_:" << socketListen << " ";
			std::cout << ip << ":" << port << "    " << NO_C << '\n';

			return (socketListen);
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
			std::vector<int>::iterator		it_ls;
			std::vector<Client>::iterator	it_client;

			FD_ZERO(&readfds); //очищаем множество
			FD_ZERO(&writefds); //очищаем множество
			max_d = 0;

			//добавляем дескриптор в множество
			for (it_ls = listenSocket.begin(); it_ls != listenSocket.end(); ++it_ls)
			{
				FD_SET((*it_ls), &readfds);
				max_d = std::max((*it_ls), max_d);
			}

			for (it_client = client.begin(); it_client != client.end(); ++it_client)
			{
				FD_SET((*it_client).getSocket(), &readfds);
				max_d = std::max((*it_client).getSocket(), max_d);
			}
		}

		void	addNewClient(fd_set &readfds)
		{
			std::vector<int>::iterator		it_ls;
			int								socket_listen;
			int								socket_client;
			struct sockaddr_in				addr_cl;
			socklen_t						addr_len;

			for (it_ls = listenSocket.begin(); it_ls != listenSocket.end(); ++it_ls)
			{
				socket_listen = (*it_ls);
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
						client.push_back(Client(socket_client, server));
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
					readSocket(*it_client);
				if (FD_ISSET(socket_client, &writefds))
					;
					//read_cl_socket(*it_client);
			}
		}

		void	readSocket(Client& client)
		{
			std::string		str;

			//читаем данные с клиентского сокета в buf____________________________
			client.readByte = recv(client.getSocket(), client.buf, BUF_SIZE - 1, 0);

			client.debagPrintReadByte();

			if (client.readByte > 0)
			{
				client.buf[client.readByte] = '\0';
				std::cout << client.buf << std::endl;
				client.request.append(client.buf);

				//request += std::string(buf);
				//std::cout << request << std::endl;
				//проверяем получили ли все данные_______________________________
				//создаем ответ и отплавляем_____________________________________
				create_response(client);
			}

			/*
			if (client.readByte < 0)
			{
				std::cout << "close fd read" << std::endl;
				shutdown(client.getSocket(), 0);
			}
			//debag_pring_request(cl_socket.fd, cl_socket.request);
			*/
		}

		void	create_response(Client& client)
		{
			client.jsn_request = client.return_map_request(client.request);

		}


	public:
		std::vector<Server>		server;
		std::vector<Client>		client;
		std::vector<int>		listenSocket;
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
