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
		Webserver()
		{
			statusCode[200] = "HTTP/1.1 200 OK\r\n";
			statusCode[400] = "HTTP/1.1 400 Bad Request\r\n";
			statusCode[404] = "HTTP/1.1 404 Not Found\r\n";
			statusCode[405] = "HTTP/1.1 405 Method Not Allowed\r\n";
			statusCode[500] = "HTTP/1.1 500 Internal Server Error\r\n";
			statusCode[501] = "HTTP/1.1 501 Not Implemented\r\n";
		}
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
			this->statusCode = oth.statusCode;
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
					socketListen = SocketListen(ip, port);
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

		int			SocketListen(const std::string& ip, const std::string& port)
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
			std::cout << BLUE << "create fd set" << RESET << std::endl;

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
			std::cout << BLUE << "add new client" << RESET << std::endl;

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

		int		checkCloseClient(Client& client)
		{
			if (client.responseHeader["Connection"] == "Connection: close\r\n")
				return (1);

			time_t		get_time;

			time(&get_time);

			if (get_time - client.timeStart > 4)
				return (1);

			return (0);
		}

		void	processingClient(fd_set &readfds, fd_set &writefds)
		{
			std::cout << BLUE << "processing client" << RESET << std::endl;
			std::vector<Client>::iterator	it_client;
			int								socket_client;

			it_client = client.begin();

			while (it_client != client.end())
			{
				socket_client = (*it_client).getSocket();

				if (FD_ISSET(socket_client, &readfds))
					readSocket(*it_client);

				else if (FD_ISSET(socket_client, &writefds))
					writeSocket(*it_client);

				if (checkCloseClient(*it_client))
				{
					debagPrintColorClient(YELLOW, socket_client);
					std::cout << YELLOW <<  " close socket" << RESET << '\n';

					close(socket_client);
					it_client = client.erase(it_client);

					continue;
				}

				++it_client;
			}
		}

		void	readSocket(Client& client)
		{
			std::cout << BLUE << "read socket" << RESET << std::endl;

			std::string		str;

			//читаем данные с клиентского сокета в buf____________________________
			client.readByte = recv(client.getSocket(), client.buf, BUF_SIZE - 1, 0);

			debagPrintColorClient(YELLOW, client.socket);
			std::cout << YELLOW << " get bytes:" << client.readByte << RESET << '\n';

			if (client.readByte > 0)
			{
				client.setTimeStart();

				client.buf[client.readByte] = '\0';
				client.request.append(client.buf);

				std::cout << client.request << std::endl;

				//проверяем получили ли все данные_______________________________
				create_response(client);
				//shutdown(client.socket, 0);
			}
		}

		void	writeSocket(Client& client)
		{
			std::cout << BLUE << "write socket" << RESET << std::endl;
			int	n;

			n = send(client.socket, client.buf_write + client.total_bytes_write, client.bytes_write, 0);
			if (-1 == n)
				std::cout << RED << "Error send" << RESET << '\n';
			else
			{
				client.total_bytes_write += n;
				client.bytes_write -= n;

				debagPrintColorClient(YELLOW, client.socket);
				std::cout << YELLOW << " write bytes:" << n << " ost bytes write:" << client.bytes_write << RESET << std::endl;

				if (client.bytes_write == 0)
					shutdown(client.socket, 1);
			}
		}


		void	create_response(Client& client)
		{
			std::cout << BLUE << "create response"<< RESET << std::endl;
			int		status_code;

			client.json_request = client.return_map_request(client.request);

			add_response_header(client);

			status_code = check_server_location(client);

			client.responseHeader["Status"] = statusCode[status_code];

			if (200 != status_code)
			{
				if (NULL != client.server)
				{
					std::string		path_file;

					path_file = client.server->errorPage[status_code];
					if (!path_file.empty())
						open_file(path_file, client);
				}
			}

			create_response_header(client);

			client.response = client.header + client.body;

			client.total_bytes_write = 0;
			client.bytes_write = client.response.size();

			client.buf_write = new char [client.bytes_write];
			strncpy(client.buf_write, client.response.c_str(), client.bytes_write);

			writeSocket(client);
		}

		//старт сздания ответа_______________________________________________________
		int		check_server_location(Client& client)
		{
			std::cout << BLUE << "check server location" << RESET << std::endl;

			client.server = find_server(client);
			if (client.server == 0)
				return (400);

			client.location = find_location(client);
			if (client.location == 0)
				return (400);

			if (check_501(client))
				return (501);

			if (check_405(client))
				return (405);

			if (client.json_request["method"] == "GET")
				return (open_file(client.path_file, client));

			return (200);
		}

		int		check_501(Client& client)
		{
			std::string	method;

			method = client.json_request["method"];

			if (method != "GET" && method != "POST" && method != "DELETE")
				return (1);
			return (0);
		}

		int		check_405(Client& client)
		{
			std::vector<std::string>::iterator	it;
			std::string	method;

			it = client.location->accessMethods.begin();

			method = client.json_request["method"];

			while (it != client.location->accessMethods.end())
			{
				if (*it == method)
					return (0);
				++it;
			}
			return (1);
		}

		/*
		void	procesing_get(Client& client)
		{
			std::cout << BLUE << "procesing get" << NO_C << std::endl;

			std::ifstream	file;
			std::string		path_file;
			std::string		line;
			std::streampos	size;
			char*			memblock;

		}
		*/

		Server*		find_server(Client& client)
		{
			std::cout << BLUE << "find server" << RESET << std::endl;

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
			std::cout << BLUE << "find location" << RESET << std::endl;

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
			std::cout << BLUE << "create path file client" << RESET << std::endl;

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

		void	add_response_header(Client& client)
		{
			if (client.json_request["Connection"].empty())
				client.responseHeader["Connection"] = "Connection: close\r\n";
			else if (client.json_request["Connection"] == "close")
				client.responseHeader["Connection"] = "Connection: close\r\n";
			else
				client.responseHeader["Connection"] = "Connection: keep-alive\r\n";

			client.responseHeader["Host"] = "Host: " + client.json_request["Host"] + "\r\n";
		}

		void	create_response_header(Client& client)
		{
			std::map<std::string, std::string>::iterator	it;

			client.header = client.responseHeader["Status"];
			client.header += client.responseHeader["Host"];

			for (it = client.responseHeader.begin(); it != client.responseHeader.end(); ++it)
			{
				if ((*it).first == "Status")
					continue;
				if ((*it).first == "Host")
					continue;
				if (!(*it).second.empty())
					client.header += (*it).second;
			}
			client.header += "\r\n";
		}

		int		open_file(const std::string& str, Client& client)
		{
			std::cout << BLUE << "open file: " << str << RESET << std::endl;

			int				length;
			char*			buffer;
			int				status_code;

			status_code = 200;

			std::ifstream	is (str, std::ifstream::binary);

			if (is)
			{
				// get length of file:
				is.seekg (0, is.end);
				length = is.tellg();
				is.seekg (0, is.beg);

				buffer = new char [length];

				debagPrintColorClient(YELLOW, client.socket);
				std::cout << YELLOW << " Reading " << length << " characters..." << RESET << '\n';

				// read data as a block:
				is.read (buffer,length);

				if (is)
				{
					client.body.assign(buffer, length);
					debagPrintColorClient(YELLOW, client.socket);
					std::cout << YELLOW << " all characters read successfully" << RESET << '\n';

					if (!client.body.empty())
					{
						client.responseHeader["Content-Length"] =
							"Content-Length: " +
							std::to_string(client.body.size()) + 
							"\r\n";
					}
				}
				else
				{
					debagPrintColorClient(RED, client.socket);
					std::cout << RED << "Error: only " << is.gcount() << " could be read" << RESET << '\n';
					status_code = 500;
				}

				is.close();
				delete [] buffer;
			}
			else
				status_code = 404;
			return (status_code);
		}


		void	debagPrintMessage(const std::string& mes)
		{ std::cout << YELLOW << get_new_time() << " " << mes << RESET << '\n'; }

		void	debagPrintColorClient(const std::string& col, const int& socket)
		{ std::cout << col << get_new_time() << " " << "cl:" << socket << RESET; }

	public:
		std::vector<Server>			server;
		std::vector<Client>			client;
		std::vector<int>			listenSocket;
		std::map<int, std::string>	statusCode;
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
