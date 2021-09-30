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
# include <fstream> //open, ifstream
# include "Server.hpp"
# include "Client.hpp"
# include <stdlib.h> //atoi

class	Webserver
{
	public:
		Webserver() {}
		Webserver(const Webserver& oth) { *this = oth; }
		~Webserver()
		{
			std::vector<int>::iterator	it;

			for (it = listenSocket.begin(); it != listenSocket.end(); ++it)
			{
				std::cout << "Webserver: Close socket listen: " << *it << std::endl;
				close (*it);
			}
		}

		Webserver&	operator= (const Webserver& oth)
		{
			this->server = oth.server;
			this->client = oth.client;
			this->listenSocket = oth.listenSocket;
			return *this;
		}

		//debag_______________________________________________________
		void		makeServer(Server& serv)
		{ server.push_back(serv); }
		//____________________________________________________________

		int			createSocketListen(void)
		{
			std::vector<Server>::iterator	it;
			std::vector<int>::iterator		it_int;
			std::string						ip;
			std::string						port;
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

		int			checkIpAddressAndPort(const std::string& ip, const std::string& port, 
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

		int			createSocketListen(const std::string& ip, const std::string& port)
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
			addr.sin_port = htons(atoi(port.c_str()));
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
				client.request.append(client.buf);

				std::cout << client.request << std::endl;

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

		void	debagPrintMessage(const std::string& mes)
		{ std::cout << YELLOW << get_new_time() << " " << mes << RESET << '\n'; }

		void	create_response(Client& client)
		{
			int		status_code;
			char	buf[33];

			client.json_request = client.return_map_request(client.request);

			status_code = check_server_location(client);

			if (200 != status_code)
			{
				if (400 == status_code)
				{
					client.header = "400 Bad Request\r\n";
					if (NULL != client.server)
					{
						std::string		path_file;

						path_file = client.server->errorPage["400"];
						if (!path_file.empty())
						{
							client.body = open_file(path_file);
							client.header += "Content-Length: ";
							client.header += std::to_string(client.body.size());
							client.header += "\r\n";
						}
					}
					client.header += "\r\n";

					client.response = client.header + client.body;
				}
			}
			else
			{
				client.response = client.header + client.body;
			}

			std::cout << client.response << std::endl;
			exit (0);
		}

		//старт сздания ответа_______________________________________________________
		int		check_server_location(Client& client)
		{
			std::cout << RED << "check_method" << NO_C << std::endl;

			client.server = find_server(client);
			if (client.server == 0)
				return (400);
				//client.header = "400 Bad Request\r\n\r\n";

			client.location = find_location(client);
			if (client.location == 0)
				return (400);
				//client.header = "400 Bad Request\r\n\r\n";

			debagPrintMessage(client.path_file);

			std::string		tmp;

			tmp = open_file(client.path_file);
			std::cout << tmp << std::endl;

			exit(0);

			if (client.json_request["method"] == "GET")
				procesing_get(client);
			else
				client.header = "501 Not Implemented\r\n\r\n";
			return (200);
		}


		Server*		find_server(Client& client)
		{
			std::cout << RED << "find server" << NO_C << std::endl;

			std::vector<Server>::iterator	it;
			std::string						host;
			std::string						ip;
			std::string						port;
			std::size_t						found;
			std::size_t						size;

			host = client.json_request["Host"];
			found = host.find(":");
			size = host.size();

			if (found != std::string::npos)
			{
				ip = host.substr(0, found);
				port = host.substr(found + 1, size - found);
			}

			for (it = server.begin(); it != server.end(); ++it)
			{
				if ((*it).ipAddress == ip && (*it).port == port)
					return (&(*it));
			}
			return (0);
		}

		Location*	find_location(Client &client)
		{
			std::cout << RED << "find location" << NO_C << std::endl;

			std::map<std::string, Location>::iterator	it;
			std::string									request_target;
			std::size_t									found;

			request_target = client.json_request["request_target"];

			while (request_target.length() != 0)
			{
				found = request_target.find_last_of("/");
				request_target = request_target.substr(0, found);

				it = client.server->location.find(request_target);
				if (it != client.server->location.end())
					return (create_path_file_client(request_target, it, client));
			}

			it = client.server->location.find("/");
			if (it != client.server->location.end())
				return (create_path_file_client("/", it, client));

			return (0);
		}

		Location*	create_path_file_client(const std::string& target, std::map<std::string, Location>::iterator it, Client& client)
		{
			std::cout << RED << "create path file client" << NO_C << std::endl;

			if (target == client.json_request["request_target"])
			{
				client.path_file = (*it).second.root;
				client.path_file += "/";
				client.path_file += (*it).second.index;
			}
			else
			{
				client.path_file = (*it).second.root;
				client.path_file += client.json_request["request_target"];
			}
			return (&(*it).second);
		}

		std::string		open_file(const std::string& str)
		{
			std::cout << RED << "open file: " << str << NO_C << std::endl;

			std::string		body;
			int				length;
			char*			buffer;

			std::ifstream	is (str, std::ifstream::binary);
			if (is)
			{
				// get length of file:
				is.seekg (0, is.end);
				length = is.tellg();
				is.seekg (0, is.beg);

				buffer = new char [length];

				std::cout << YELLOW << get_new_time() << " ";
				std::cout << "Reading " << length << " characters... ";
				std::cout << RESET << '\n';

				// read data as a block:
				is.read (buffer,length);


				if (is)
				{
					body.assign(buffer, length);
					debagPrintMessage("all characters read successfully");
				}
				else
				{
					std::cout << RED << get_new_time() << " ";
					std::cout << "Error: only " << is.gcount() << " could be read";
					std::cout << RESET << '\n';
				}

				is.close();
			    // ...buffer contains the entire file...
				delete [] buffer;
			}
			return (body);
		}


		void	procesing_get(Client& client)
		{
			std::cout << RED << "procesing_get" << NO_C << std::endl;
			std::ifstream	file;
			std::string		path_file;
			std::string		line;
			std::streampos	size;
			char*			memblock;

		}
			/*
			path_file = client.json_request["request_target"];
			if (path_file == "/")
				path_file = "www/index.nginx.html";

			file.open (path_file, std::ios::in | std::ios::binary);

			if (file.is_open())
			{
				while (getline(file, line))
				{
					body += line;
					body += "\n";
				}
				file.close();

				char	byt[33];

				itoa (3, byt, 10);
				std::cout << byt << std::endl;

				create_header_200();
				std::cout << RED << "is_open" << NO_C << std::endl;
				size = file.tellg();

				std::cout << "size= " << size << std::endl;
				memblock = new char [size];

				file.seekg (0, std::ios::beg);
				file.read (memblock, size);

				body = std::string(memblock, size);
				std::cout << "debeg body\n\n\n";
				std::cout << memblock << std::endl;

				delete [] memblock;
			}
			else
				header = "404 Not Found\r\n\r\n";
		}
		*/
		/*
		void	create_header_200(void)
		{
			header = "HTTP/1.1 200 OK\r\n";
			header += "Host: ";
			header += server.getIpAddress();
			header += ":";
			header += server.getPort();
			header += "\r\n";
			header += "Content-Type: text/html; charset=UTF-8\r\n";
			header += "Connection: close\r\n";
			header += "Content-Length: ";
			header += body.length();
			header += "\r\n\r\n";
		}

		int		check_method(const std::string& m)
		{
			std::vector<std::string>			method(3);
			std::vector<std::string>::iterator	it;

			method.push_back("GET");
			method.push_back("POST");
			method.push_back("DELETE");

			for (it = method.begin(); it != method.end(); ++it)
			{
				if (*it == m)
					return (1);
			}
			return (-1);
		}
		*/


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
