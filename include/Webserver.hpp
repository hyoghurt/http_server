#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# define TIME_KEEP_ALIVE 5

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
# include <dirent.h> //opendir
# include <sys/stat.h> //stat or <sys/types.h> <unistd.h>

class	Webserver
{
	public:
		Webserver()
		{
			statusCode[200] = "200 OK";
			statusCode[201] = "201 Created";
			statusCode[204] = "204 No Content";
			statusCode[400] = "400 Bad Request";
			statusCode[404] = "404 Not Found";
			statusCode[405] = "405 Method Not Allowed";
			statusCode[413] = "413 Payload Too Large";
			statusCode[500] = "500 Internal Server Error";
			statusCode[501] = "501 Not Implemented";

			interpreter[".py"] = "/Users/hyoghurt/.brew/bin/python3";
			interpreter[".php"] = "/usr/bin/php";
			interpreter[".perl"] = "/usr/bin/perl";
			interpreter[".rb"] = "/usr/bin/ruby";
			interpreter[".rbw"] = "/usr/bin/ruby";
			interpreter[".sh"] = "/bin/sh";
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

		/*
		std::vector<std::string>	split_string(std::string s, const std::string& delimiter)
		{
			std::vector<std::string>	result;
			std::string					token;
			size_t						pos = 0;

			while ((pos = s.find(delimiter)) != std::string::npos)
			{
			    token = s.substr(0, pos);
				result.push_back(token);
			    s.erase(0, pos + delimiter.length());
			}
			result.push_back(s);
			return (result);
		}

		void	inter(void)
		{
			std::string							path_;
			std::vector<std::string>			token;
			std::vector<std::string>::iterator	it;

			path_ = getenv("PATH");
			token = split_string(path_, ":");

			for (it = token.begin(); it != token.end(); ++it)
				std::cout << (*it) << std::endl;

			//exit(0);
		}
		*/

		//debug_______________________________________________________
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
						client.push_back(Client(socket_client, inet_ntoa(addr_cl.sin_addr)));
					}
				}
			}
		}

		int		checkCloseClient(Client& client)
		{
			time_t		get_time;

			if (client.responseHeader["Connection"] == "Connection: close\r\n")
				return (1);

			time(&get_time);
			if (get_time - client.timeStart > TIME_KEEP_ALIVE)
				return (1);

			return (0);
		}

		void	processingClient(fd_set &readfds, fd_set &writefds)
		{
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
					(*it_client).debug_info("close socket");

					close(socket_client);
					it_client = client.erase(it_client);
					continue;
				}
				++it_client;
			}
		}

		void	readSocket(Client& client)
		{
			int				bytes, found, found_2;
			std::string		len;

			bytes = recv(client.getSocket(), client.buf, BUF_SIZE - 1, 0);

			if (bytes > 0)
			{
				client.setTimeStart();

				client.debug_info("get bytes: " + std::to_string(bytes));

				client.buf[bytes] = '\0';
				client.request.append(client.buf);

				if (client.readByte == 0)
				{
					if (client.request.find("\r\n\r\n") != std::string::npos)
					{
						found = client.request.find("Content-Length:");
						if (found != std::string::npos)
						{
							found_2 = client.request.find("\r\n", found);
							len = client.request.substr(found + 16, found_2 - (found + 16));
							client.readByte = atoi(len.c_str());
						}
						client.readByte += client.request.find("\r\n\r\n") + 4;
					}
				}
				if (client.request.size() == client.readByte)
				{
					std::cout << client.request << std::endl;

					client.readByte = 0;
					create_response(client);
					shutdown(client.socket, 0);
				}
			}
		}

		void	writeSocket(Client& client)
		{
			int		n;

			n = send(client.socket, client.response.c_str(), client.response.size(), 0);
			if (-1 == n)
				print_error("Error send");
			else
			{
				client.response.erase(0, n);

				client.debug_info("write bytes: " + std::to_string(n) + " ost bytes:" + std::to_string(client.response.size()));

				if (client.response.size() == 0)
					shutdown(client.socket, 1);
			}
		}

		void	create_response(Client& client)
		{
			print_debug("F create response");

			int				status_code;
			std::string		path_file;

			client.create_json_request();
			client.request.clear();

			status_code = processing_request(client);
			client.responseHeader["Status"] = statusCode[status_code];

			if (200 != status_code)
			{
				if (NULL != client.server)
				{
					path_file = client.server->errorPage[status_code];
					if (!path_file.empty())
						client.open_file(path_file);
				}
			}

			client.response_total();
			std::cout << client.response << std::endl;

			writeSocket(client);
			//exit(0);
		}

		//старт сздания ответа_______________________________________________________
		int		processing_request(Client& client)
		{
			std::map<std::string, std::string>::iterator	it;

			client.server = find_server(client);
			if (client.server == 0)
				return (400);

			if (client.check_413())
				return (413);

			client.location = find_location(client);
			if (client.location == 0)
				return (400);

			if (client.check_501())
				return (501);

			if (client.check_405())
				return (405);

			if (client.json_request["method"] == "DELETE")
				return (client.deleteFile());

			for (it = interpreter.begin(); it != interpreter.end(); ++it)
			{
				if (client.path_file.find((*it).first) != std::string::npos)
				{
					client.interpreter = (*it).second;
					return (client.cgi_run());
				}
			}

			if (client.json_request["method"] == "GET")
				return (client.get_run());

			return (client.post_run());
		}

		Server*		find_server(Client& client)
		{
			print_debug("F find server");

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
			std::map<std::string, Location>::iterator	it;
			std::string									request_target;
			int											found;

			request_target = client.json_request["request_target"];

			print_debug("F find loacation for target: [" + request_target + "]");

			found = request_target.find("?");
			if (std::string::npos != found)
				request_target.erase(found);

			while (request_target.length() != 0)
			{
				it = client.server->location.find(request_target);
				if (it != client.server->location.end())
				{
					parser_url(it, client);
					return (&(*it).second);
				}

				if (request_target == "/")
					return (0);

				found = request_target.find_last_of("/");
				if (std::string::npos == found)
					return (0);

				if (found == 0)
					found = 1;

				request_target.erase(found);
			}
			return (0);
		}

		void	parser_url(std::map<std::string, Location>::iterator it, Client& client)
		{
			print_debug("F create client.path_file (location: [" + (*it).first + "])");

			int				found;
			std::string		request_target = client.json_request["request_target"];

			if ((*it).first != "/")
				request_target.erase(0, (*it).first.size());

			client.path_file = (*it).second.root + request_target;

			found = client.path_file.find('?');
			if (std::string::npos != found)
			{
				client.envCgi["QUERY_STRING"] = client.path_file.substr(found + 1);
				client.path_file.erase(found);
			}

			found = client.path_file.find('.');
			if (std::string::npos != found)
			{
				found = client.path_file.find('/', found);
				if (std::string::npos != found)
				{
					client.envCgi["PATH_INFO"] = client.path_file.substr(found);
					client.path_file.erase(found);
				}
			}
			else
			{
				if (1 == client.check_dir_or_file(client.path_file))
				{
					if (client.path_file.find_last_of('/') != (client.path_file.size() - 1))
						client.path_file += "/";
					client.path_file += (*it).second.index;
				}
			}
			client.debug_info("client.path_file:" + client.path_file);
		}

		void	debugPrintMessage(const std::string& mes)
		{ std::cout << YELLOW << get_new_time() << " " << mes << RESET << '\n'; }

		void	debugPrintColorClient(const std::string& col, const int& socket)
		{ std::cout << col << get_new_time() << " " << "cl:" << socket << RESET; }

	public:
		std::vector<Server>					server;
		std::vector<Client>					client;
		std::vector<int>					listenSocket;
		std::map<int, std::string>			statusCode;
		std::map<std::string, std::string>	interpreter;
};

/*
void			print_error(const std::string& str);
int				create_listen_socket(void);
std::string		ret_str_open(const std::string& file);
void			debug_pring_request(const int& fd_client, const std::string& str);
std::string		get_new_time(void);
void			print_connect_info(int fd_cl, struct sockaddr_in addr_cl);
void			read_cl_socket(Cl_socket cl_socket);
void			create_fd_set(int &max_d, int &ls, std::vector<Cl_socket> &store, fd_set &readfds);
void			add_fd_store(int &ls, std::vector<Cl_socket> &store);
*/

#endif
