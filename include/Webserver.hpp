#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <iostream>
# include <vector>
# include <fstream> //open, ifstream
# include <stdlib.h> //atoi
# include <dirent.h> //opendir
# include <sys/stat.h> //stat or <sys/types.h> <unistd.h>
# include <sstream> //std::istringstream
# include <csignal>

# include "Server.hpp"
# include "Client.hpp"

class	Webserver
{
	public:
//CONSTRUCTOR__________________________________________________________________
		Webserver() {}
//COPY_________________________________________________________________________
		Webserver(const Webserver& oth) { *this = oth; }
//DESTRUCTOR___________________________________________________________________
		~Webserver()
		{
			std::vector<int>::iterator	it;

			for (it = listenSocket.begin(); it != listenSocket.end(); ++it)
			{
				print_info("Webserver: Close socket listen: "
						+ std::to_string(*it));
				close (*it);
			}
		}
//OPERATOR=____________________________________________________________________
		Webserver&	operator= (const Webserver& oth)
		{
			this->server = oth.server;
			this->client = oth.client;
			this->listenSocket = oth.listenSocket;
			return *this;
		}
//CONF_PARSER__________________________________________________________________
		void	setServer(const Server& serv)	{ server.push_back(serv); }
		void	debug_show_conf();
		int		readConfigFile(const char* fileName);
		void	add_client_max_body_size();
		bool	checkСorrectField();
		bool	checkCorrectMethodName(std::vector <std::string> names);
		bool	checkCorrectIP(std::string ip);
		bool 	checkCorrectHost(std::string host);

//CREATE_LISTEN_SOCKET_________________________________________________________
		int		createSocketListen()
		{
			std::vector<Server>::iterator	it;
			std::string						ip;
			std::string						port;
			int								socketListen;

			for (it = server.begin(); it != server.end(); ++it)
			{
				ip = (*it).getIpAddress();
				port = (*it).getPort();

				if (-1 != checkIpAddressAndPort(ip, port, it))
				{
					socketListen = createSListen(ip, port);
					if (-1 == socketListen)
						return (-1);
					listenSocket.push_back(socketListen);
				}
			}
			return (0);
		}
//CREATE_LISTEN_SOCKET_________________________________________________________
		int		checkIpAddressAndPort (const std::string& ip,
				const std::string& port, std::vector<Server>::iterator it_end)
		{
			std::vector<Server>::iterator	it;

			for (it = server.begin(); it != it_end; ++it)
				if ((*it).getIpAddress() == ip and (*it).getPort() == port)
					return (print_error("check ip and port"));
			return (0);
		}
//CREATE_LISTEN_SOCKET_________________________________________________________
		int		createSListen(const std::string& ip, const std::string& port)
		{
			//создание сокета__________________________________________________
			int	socketListen = socket(AF_INET, SOCK_STREAM, 0);
			if (socketListen == -1)
				return (print_error("socket"));

			//перевод сокета в неблокирующий режим_____________________________
			int	flags = fcntl(socketListen, F_GETFL);
			if (-1 == fcntl(socketListen, F_SETFL, flags | O_NONBLOCK))
				return (print_error("fcntl"));

			//установили флаг, для предотвращения залипаниz TCP порта__________
			int	opt = 1;
			if (-1 == setsockopt(socketListen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
				return (print_error("setsockopt"));

			//сопостовление созданному сокету конкретного адреса_______________
			struct sockaddr_in  addr;

			addr.sin_family = AF_INET;
			addr.sin_port = htons(atoi(port.c_str()));
			addr.sin_addr.s_addr = inet_addr(ip.c_str());
			if (-1 == bind(socketListen,(struct sockaddr*)&addr, sizeof(addr)))
				return (print_error("bind"));

			//переводим сокет в слушаюший режим________________________________
			if (-1 == listen(socketListen, 128))
				return (print_error("listen"));

			//печать на консоль информацию______________________________________
			std::cout << PINK << "\n__START SERVER: SOCKET:" << socketListen;
			std::cout << " " << ip << ":" << port << "    " << NO_C << '\n';
			return (socketListen);
		}

		static void	signal_handler(int sig)
		{
			//this->~Webserver();
		}
//START_WEBSERVER______________________________________________________________
		int		start()
		{
			fd_set		readfds;
			fd_set		writefds;
			int			max_d;

			//std::signal(SIGINT, Webserver::signal_handler);

			//мультиплексирование ввода-вывода select() poll() kqueue()________
			while (1)
			{
				//создаем множество____________________________________________
				createFdSet(max_d, readfds, writefds);
				//создаем выборку файловых дескрипторов________________________
				if (-1 == select(max_d + 1, &readfds, &writefds, NULL, NULL))
					return (print_error("select"));
				//запрос от нового клиента_____________________________________
				addNewClient(readfds);
				//данные клиента_______________________________________________
				processingClient(readfds, writefds);
			}
			return (0);
		}
//CREATE_FD_SET________________________________________________________________
		void	createFdSet(int &max_d, fd_set &readfds, fd_set &writefds)
		{
			std::vector<int>::iterator		ils;
			std::vector<Client>::iterator	icl;

			FD_ZERO(&readfds);	//очищаем множество
			FD_ZERO(&writefds);	//очищаем множество
			max_d = 0;

			//добавляем дескриптор в множество_________________________________
			for (ils = listenSocket.begin(); ils != listenSocket.end(); ++ils)
			{
				FD_SET((*ils), &readfds);
				max_d = std::max((*ils), max_d);
			}

			for (icl = client.begin(); icl != client.end(); ++icl)
			{
				FD_SET((*icl).getSocket(), &writefds);
				FD_SET((*icl).getSocket(), &readfds);

				max_d = std::max((*icl).getSocket(), max_d);
			}
		}
//ADD_NEW_CLIENT_______________________________________________________________
		void	addNewClient(fd_set &readfds)
		{
			std::vector<int>::iterator		ils;
			int								scl;
			struct sockaddr_in				acl;
			socklen_t						aln;

			for (ils = listenSocket.begin(); ils != listenSocket.end(); ++ils)
			{
				if (FD_ISSET((*ils), &readfds))
				{
					//получаем сокет для связи с клиентом______________________
					scl = accept((*ils), (struct sockaddr*) &acl, &aln);

					if (scl == -1)
						print_error("accept");
					else
					{
						fcntl(scl, F_SETFL, O_NONBLOCK);
						print_connect_info((*ils), scl, acl);
						client.push_back(Client(scl, inet_ntoa(acl.sin_addr)));
					}
				}
			}
		}
//PROCESSING_CLIENT____________________________________________________________
		void	processingClient(fd_set &readfds, fd_set &writefds)
		{
			std::vector<Client>::iterator	it_client = client.begin();
			int								socket_client;
			int								bytes(0);

			while (it_client != client.end())
			{
				socket_client = (*it_client).getSocket();

				//данные_для_чтения____________________________________________
				if (FD_ISSET(socket_client, &readfds))
					bytes = readSocket(*it_client);

				if (bytes < 0)
				{
					(*it_client).debug_info("close socket (read = -1)");
					close(socket_client);
					it_client = client.erase(it_client);
					continue;
				}

				//данные_для_отправки__________________________________________
				if (FD_ISSET(socket_client, &writefds))
					bytes = writeSocket(*it_client);

				//закрытие_клиента_____________________________________________
				if (bytes < 0 || checkCloseClient(*it_client))
				{
					if (bytes < 0)
						(*it_client).debug_info("close socket (write = -1)");
					else
						(*it_client).debug_info("close socket (time > "
								+ std::to_string(TIME_KEEP_ALIVE) + ")");
					close(socket_client);
					it_client = client.erase(it_client);
					continue;
				}
				++it_client;
			}
		}
//READ_SOCKET__________________________________________________________________
		int		readSocket(Client& client)
		{
			//чтение_данных_от_клиента_________________________________________
			int	bytes = recv(client.getSocket(), client.buf, BUF_SIZE, 0);

			if (bytes > 0)
			{
				client.setTimeStart();
				client.request.append(client.buf, bytes);

				if (client.readByte == 0 && client.chunked == 0)
					client.erase_header_request();
				if (client.chunked > 0)
					client.exec_chunk();

				//получили_данные_полностью____________________________________
				if (client.readSocketCheckEndRead())
					create_response(client, 0);
			}
			return (bytes);
		}
//WRITE_SOCKET_________________________________________________________________
		int		writeSocket(Client& client)
		{
			int		bytes(0);

			if (client.flag)
			{
				client.setTimeStart();

				bytes = write(client.socket, client.response.c_str(),
						client.response.size());

				if (bytes > 0)
				{
					client.response.erase(0, bytes);
					if (client.response.empty())
					{
						client.flag = 0;
						client.debug_info(client.responseHeader["Status"]);
					}
				}
			}
			return (bytes);
		}
//RESPONSE_____________________________________________________________________
		void	create_response(Client& client, const int& max_header)
		{
			int				status_code;
			std::string		path_file;

			//выполнение_запроса_______________________________________________
			if (max_header == 413)
				status_code = 413;
			else
				status_code = processing_response(client);

			//формирование_итога_для_отправки__________________________________
			client.setResponseHeaderStatus(status_code);
			if (200 != status_code)
			{
				path_file = client.getPathErrorPage(status_code);
				if (!path_file.empty())
					client.open_file(path_file);
			}
			client.response_total();
			client.flag = 1;
		}
//RESPONSE_____________________________________________________________________
		int		processing_response(Client& client)
		{
			std::map<std::string, std::string>::iterator	it;

			//привязка_к_config_server_________________________________________
			if (find_server(client))
				return (400);

			//вытащить_переменные_для_cgi______________________________________
			client.find_query_string_path_info();

			//привязка_к_config_location_______________________________________
			if (client.find_location())
				return (400);

			//редирект_________________________________________________________
			if (client.check_redirect())
				return (client.getLocation()->getReturnCode());

			//проверка_размера_тела_запроса____________________________________
			if (client.check_413())
				return (413);

			//проверка_метода_(GET,POST,DELETE)________________________________
			if (client.check_501())
				return (501);

			//проверка_метода_допустимых_в_config______________________________
			if (client.check_405())
				return (405);

			//выполнение_метода_DELETE_________________________________________
			if (client.getRequestMethod() == "DELETE")
				return (client.deleteFile());

			//выполнение_скачивания____________________________________________

			//выполнение_CGI___________________________________________________
			if (!client.getLocation()->getCgiPass().empty())
				return (client.cgi_run());

			//выполнение_метода_GET____________________________________________
			if (client.getRequestMethod() == "GET")
				return (client.get_run());

			//выполнение_метода_POST___________________________________________
			return (client.post_run());
		}
//CONFIG_SERVER________________________________________________________________
		int		find_server(Client& client)
		{
			std::vector<Server>::iterator	it;
			std::string						host;
			std::string						port;
			size_t							found;

			client.setServer(nullptr);

			host = client.getRequestHost();
			found = host.find(":");

			if (found != std::string::npos)
			{
				port = host.substr(found + 1);
				host.erase(found);
			}

			for (it = server.begin(); it != server.end(); ++it)
				if ((*it).getIpAddress() == host && (*it).getPort() == port)
					client.setServer(&(*it));

			for (it = server.begin(); it != server.end(); ++it)
				if ((*it).getServerName() == host)
					client.setServer(&(*it));

			if (client.getServer() == nullptr)
				return (1);
			return (0);
		}
//CHECK_CLOSE_CLIENT___________________________________________________________
		bool	checkCloseClient(Client& client)
		{
			time_t			get_time;

			if (client.flag)
				return (false);
			if (client.checkResponseHeaderConnectionClose())
				return (true);
			if (time(&get_time) - client.timeStart > TIME_KEEP_ALIVE)
				return (true);
			return (false);
		}

	public:
		std::vector<Server>					server;
		std::vector<Client>					client;
		std::vector<int>					listenSocket;
};

#endif
