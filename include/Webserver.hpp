#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# define CONF_DEFAULT "conf.conf"

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


/*
 * +	check write and read -1 and 0
 * +	переписать автоиндекс
 * +	переписать нахождение локатион
 * +	content-type: png, html
 * +	скачать то Content-Type: application/octet-stream
 * +	написат redirect
 * +	использовать server_name
 * +	проверить автоиндекс / в конце
 * +	подисать cgi post
 * +	исправить тест с размером тела 100000000 и отправки его в cgi
 * +	исправить код ответа при создании файла
 *
 *
 *
 * подисать cgi окружение
 * написать signal для выхода из программы
 * запустить tester
 * запустить siege
 * корректировка парсера полученного хедера (пробелы и перенос строки)
 *
 * _________CURL____________________________________________________________
 * curl --response host:port:address http://host:port
 *
 * POST
 * curl -d "text" http://127.0.0.1:9000/dowloads/file
 *
 * вывести только заголовок
 * curl -I https://losst.ru
 *
 * вывести загаловок и тело
 * curl -i https://losst.ru
 *
 * отправить метод
 * curl -X GET
 * curl -X POST -d @file.txt http://127.0.0.1:9000/dowloads/file
 * curl -X POST -d "text" http://127.0.0.1:9000/dowloads/file
 *
 * отправить заголовок
 * curl -I --header 'If-Modified-Since: Mon, 26 Dec 2016 18:13:12 GMT' https://losst.ru
 *
 *
 * ________________________TEST_________________________________________________
 * curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit"
 * in the configuration try to setup the same port multiple times. it should not work
*/

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
			std::cout << "destructor webserver" << std::endl;

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
		void			debug_show_conf();
		int				readConfigFile(const char* fileName);
//CREATE_LISTEN_SOCKET_________________________________________________________
		int			createSocketListen ()
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
					socketListen = SocketListen(ip, port);
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
					return (-1);
			return (0);
		}
//CREATE_LISTEN_SOCKET_________________________________________________________
		int		SocketListen(const std::string& ip, const std::string& port)
		{
			int	socketListen;

			//создание сокета__________________________________________________
			socketListen = socket(AF_INET, SOCK_STREAM, 0);
			if (socketListen == -1)
			{
				print_error("socket");
				return (-1);
			}

			//перевод сокета в неблокирующий режим_____________________________
			int	flags = fcntl(socketListen, F_GETFL);
			if (-1 == fcntl(socketListen, F_SETFL, flags | O_NONBLOCK))
			{
				print_error("fcntl");
				return (-1);
			}

			//установили флаг, для предотвращения залипаниz TCP порта__________
			int	opt = 1;
			if (-1 == setsockopt(socketListen, SOL_SOCKET,
						SO_REUSEADDR, &opt, sizeof(opt)))
			{
				print_error("setsockopt");
				return (-1);
			}

			//сопостовление созданному сокету конкретного адреса_______________
			struct sockaddr_in  addr;

			addr.sin_family = AF_INET;
			addr.sin_port = htons(atoi(port.c_str()));
			addr.sin_addr.s_addr = inet_addr(ip.c_str());
			if (-1 == bind(socketListen,(struct sockaddr*)&addr, sizeof(addr)))
			{
				print_error("bind");
				return (-1);
			}

			//переводим сокет в слушаюший режим________________________________
			if (-1 == listen(socketListen, 128))
			{
				print_error("listen");
				return (-1);
			}

			//печать на консоль информацию______________________________________
			std::cout << PINK << "\n_____START SERVER_:" << socketListen << " ";
			std::cout << ip << ":" << port << "    " << NO_C << '\n';

			return (socketListen);
		}

		void	add_client_size()
		{
			std::vector<Server>::iterator		it;
			std::vector<Location>::iterator		lc;

			for (it = server.begin(); it != server.end(); ++it)
			{
				for (lc = (*it).location.begin(); lc != (*it).location.end(); ++lc)
				{
					if ((*lc).clientMaxBodySize == -1)
						(*lc).clientMaxBodySize = (*it).clientMaxBodySize;
				}
			}
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
			int			num(0);

			//std::signal(SIGINT, Webserver::signal_handler);

			add_client_size();

			//мультиплексирование ввода-вывода select() poll() kqueue()________
			while (1)
			{
				//создаем множество____________________________________________
				createFdSet(max_d, readfds, writefds);
				//создаем выборку файловых дескрипторов________________________
				if (1 > select(max_d + 1, &readfds, &writefds, NULL, NULL))
				{
					print_error("select");
					if (num++ == 20)
						return (-1);
					usleep(500000);
					continue ;
				}
				num = 0;
				//запрос от нового клиента_____________________________________
				addNewClient(readfds);
				//данные клиента_______________________________________________
				processingClient(readfds, writefds);
			}
			std::cout << "end loop" << std::endl;
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
				//else
				if ((*icl).flag == 0)
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
			std::vector<Client>::iterator	it_client;
			int								socket_client;
			int								bytes(0);

			it_client = client.begin();

			while (it_client != client.end())
			{
				socket_client = (*it_client).getSocket();
				bytes = 0;

				//данные_для_отправки__________________________________________
				if ((*it_client).flag && FD_ISSET(socket_client, &writefds))
					bytes = writeSocket(*it_client);

				//данные_для_чтения____________________________________________
				if (FD_ISSET(socket_client, &readfds))
					bytes = readSocket(*it_client);

				//закрытие_клиента_____________________________________________
				if (bytes < 0 || checkCloseClient(*it_client))
				{
					std::cout << "bytes=[" << bytes << "]check=[" << checkCloseClient(*it_client) << "]" << std::endl;

					(*it_client).debug_info("close socket");
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
			int	bytes = recv(client.getSocket(), client.buf, BUF_SIZE - 1, 0);

			if (bytes > 0)
			{
				//client.debug_info("get bytes: " + std::to_string(bytes));
				client.setTimeStart();
				client.request.append(client.buf, bytes);

				if (client.readByte == 0 && client.chunked == 0)
					erase_header_request(client);
				if (client.chunked > 0)
					exec_chunk(client);

				//получили_данные_полностью____________________________________
				if (client.chunked == 0 && client.request.size() == client.readByte)
				{
					client.readByte = 0;

					if (client.json_request.find("Content-Length") != client.json_request.end())
						client.json_request["body"] = client.request;

					/*
					std::cout << "-START-RESPONSE-CREATE-------------------" << std::endl;
					std::cout << "BODY SIZE = " << client.json_request["body"].size() << std::endl;
					std::cout << "-----------------------------------------" << std::endl;
					*/

					create_response(client, 0);
					//закрытие_сокета_чтения___________________________________
					//shutdown(client.socket, 0);
				}
			}
			return (bytes);
		}
//READ_SOCKET__________________________________________________________________
		void	erase_header_request(Client& client)
		{
			std::map<std::string, std::string>::iterator	it;
			size_t											found;

			found = client.request.find("\r\n\r\n");

			if (found != std::string::npos)
			{
				/*
				client.debug_info("REQUEST");
				std::cout << "----------------------------\n";
				std::cout << client.request << std::endl;
				std::cout << "----------------------------\n";
				*/

				client.create_json_request_header(client.request.substr(0, found));
				client.request.erase(0, found + 4);

				it = client.json_request.find("Content-Length");
				if (it != client.json_request.end())
					client.readByte = atoi((*it).second.c_str());

				it = client.json_request.find("Transfer-Encoding");
				if (it != client.json_request.end() && (*it).second == "chunked")
					client.chunked = 1;
			}
		}
//READ_SOCKET__________________________________________________________________
		void	exec_chunk(Client& client)
		{
			size_t				found;

			while (client.chunked > 0)
			{
				if (client.chunked == 1)
				{
					found = client.request.find("\r\n");
					if (found != std::string::npos)
					{
						client.readByte = convert_str_to_base16(client.request.substr(0, found));
						client.chunked = 2;
						client.request.erase(0, found + 2);
					}
					else
					{
						break ;
					}
				}

				if (client.chunked == 2)
				{
					if (client.request.size() >= client.readByte + 2)
					{
						client.json_request["body"] += client.request.substr(0, client.readByte);
						client.request.erase(0, client.readByte + 2);

						if (client.readByte == 0)
							client.chunked = 0;
						else
							client.chunked = 1;
					}
					else
					{
						break ;
					}
				}
			}
		}

		int	check_chunk_header_response(Client& client)
		{
			std::map<std::string, std::string>::iterator	it;

			it = client.responseHeader.find("Transfer-Encoding");
			if (it != client.responseHeader.end() && (*it).second == "chunked")
				return (1);
			return (0);
		}
//WRITE_SOCKET_________________________________________________________________
		int		writeSocket(Client& client)
		{
			int				bytes;
			std::string		tmp;

			/*
			if (client.response.size() > 50000)
			{
				tmp = client.response.substr(0, 50000);
				bytes = send(client.socket, tmp.c_str(), tmp.size(), 0);
				if (bytes > 0)
				{
					client.response.erase(0, bytes);
				}
			}
			else if (!client.response.empty())
			{
				tmp = client.response.substr();
				bytes = send(client.socket, tmp.c_str(), tmp.size(), 0);
				if (bytes > 0)
				{
					client.response.erase(0, bytes);
				}
			}
			if (client.response.empty())
			{
				client.flag = 0;
			}
			*/
			/*
			int bytes = send(client.socket, client.response.c_str(),
					client.response.size(), 0);
					*/

			if (client.flag)
			{
				bytes = write(client.socket, client.response.c_str(),
						client.response.size());

				if (bytes > 0)
				{
					client.setTimeStart();

					client.response.erase(0, bytes);
					if (client.response.empty())
					{
						client.debug_info("write end");
						client.flag = 0;
					}
				}

				/*
				client.debug_info("write bytes: " + std::to_string(bytes)
						+ " ost bytes:" + std::to_string(client.response.size()));
						*/
			}

			return (bytes);
		}
//RESPONSE_____________________________________________________________________
		void	create_response(Client& client, const int& max_header)
		{
			print_debug("F create response");

			int				status_code;
			std::string		path_file;

			client.responseHeader.clear();

			std::cout << "____REQUEST_HEADER________\n";
			client.debug_show_map(client.json_request);
			std::cout << "__________________________\n";

			//выполнение_запроса_______________________________________________
			if (max_header == 413)
				status_code = 413;
			else
				status_code = processing_request(client);

			//формирование_итога_для_отправки__________________________________
			client.responseHeader["Status"] = get_status_code(status_code);
			if (200 != status_code)
			{
				if (client.server != nullptr)
				{
					path_file = client.server->errorPage[status_code];
					if (!path_file.empty())
						client.open_file(path_file);
				}
			}
			client.response_total();

			std::cout << "____TOTAL HEADER__________\n";
			std::cout << client.header << std::endl;
			std::cout << "__________________________\n";

			client.header.clear();
			client.flag = 1;

			//отправка_данных_клиенту__________________________________________
			writeSocket(client);
		}
//RESPONSE_____________________________________________________________________
		int		processing_request(Client& client)
		{
			std::map<std::string, std::string>::iterator	it;

			//привязка_к_config_server_________________________________________
			client.server = find_server(client);
			if (client.server == 0)
				return (400);

			//вытащить_переменные_для_cgi______________________________________
			client.find_query_string_path_info();

			//привязка_к_config_location_______________________________________
			int	status = find_location(client);

			if (status == 400)
				return (400);

			if (status == 301)
				return (301);

			//редирект_________________________________________________________
			if (client.check_redirect())
				return (client.location->return_code);

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
			if (client.json_request["method"] == "DELETE")
				return (client.deleteFile());

			//выполнение_скачивания____________________________________________
			if (client.location->rule == "/dowloads")
				if (client.json_request["method"] == "GET")
					return (client.get_run());

			//выполнение_CGI___________________________________________________
			if (!client.location->cgiPass.empty())
				return (client.cgi_run());

			//выполнение_метода_GET____________________________________________
			if (client.json_request["method"] == "GET")
				return (client.get_run());

			//выполнение_метода_POST___________________________________________
			return (client.post_run());
		}
//CONFIG_SERVER________________________________________________________________
		Server*		find_server(Client& client)
		{
			print_debug("F find server");

			std::vector<Server>::iterator	it;
			std::string						host;
			std::string						port;
			std::size_t						found;

			host = client.json_request["Host"];
			found = host.find(":");

			if (found != std::string::npos)
			{
				port = host.substr(found + 1);
				host.erase(found);
			}

			for (it = server.begin(); it != server.end(); ++it)
				if ((*it).ipAddress == host && (*it).port == port)
					return (&(*it));

			for (it = server.begin(); it != server.end(); ++it)
				if ((*it).serverName == host)
					return (&(*it));

			return (0);
		}
//CONFIG_LOCATION______________________________________________________________
		int			find_location(Client& client)
		{
			int		header_location(0);

			client.location = nullptr;

			//find_location_filename_extension(client);

			if (client.location == nullptr)
			{
				find_location_directory(client);

				if (client.location != nullptr)
				{
					if (client.location->rule != "/")
						client.path_file.erase(0, client.location->rule.size());
					client.path_file = client.location->root + client.path_file;

					int	d = check_dir_or_file(client.path_file);
					if (d == 1)
					{
						int	size = client.path_file.size();
						if (client.path_file[size - 1] != '/')
						{
							client.path_file.push_back('/');
							header_location = 1;
							//return (301);
						}

						d = check_dir_or_file(client.path_file + client.location->index);
						/*
						if (d != -1 && !client.location->autoindex)
							client.path_file.append(client.location->index);
							*/
						if (d == -1)
						{
							if (!client.location->autoindex)
							{
								client.path_file.append(client.location->index);
								if (header_location)
									client.responseHeader["Location"] = client.json_request["request_target"] + "/" + client.location->index;
							}
						}
						if (d == 0)
						{
							client.path_file.append(client.location->index);
							if (header_location)
								client.responseHeader["Location"] = client.json_request["request_target"] + "/" + client.location->index;
						}
					}
					find_location_filename_extension(client);
				}
			}
			if (client.location == nullptr)
				return (400);

			client.debug_info("client.path_file:" + client.path_file);
			return (0);
		}
//CONFIG_LOCATION______________________________________________________________
		int		find_location_directory(Client &client)
		{
			std::string		request_target;
			int				found;

			request_target = client.path_file;
			print_debug("F find loacation for target: ["+request_target+"]");

			while (request_target.length() != 0)
			{
				client.location = return_location(client, request_target);

				if (client.location != nullptr)
					return (0);

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
//CONFIG_LOCATION______________________________________________________________
		Location*	return_location(Client& client, const std::string& str)
		{
			std::vector<Location>::iterator	it;

			it = client.server->location.begin();
			while (it != client.server->location.end())
			{
				if ((*it).rule == str)
					return (&(*it));
				++it;
			}

			return (nullptr);
		}
//CONFIG_LOCATION______________________________________________________________
		void	find_location_filename_extension(Client& client)
		{
			std::string		request_target;
			size_t			found;
			Location*		tmp_loc;

			print_debug("F find loacation for target: ["+client.path_file+"]");

			found = client.path_file.find_last_of('.');
			if (found != std::string::npos)
			{
				request_target = client.path_file.substr(found);
				tmp_loc = return_location(client, request_target);
				if (tmp_loc != nullptr)
					client.location = tmp_loc;
			}
		}
//CHECK_CLOSE_CLIENT___________________________________________________________
		int		checkCloseClient(Client& client)
		{
			time_t											get_time;
			std::map<std::string, std::string>::iterator	it;

			/*
			if (client.flag)
				return (0);
				*/

			it = client.responseHeader.find("Connection");
			if (it != client.responseHeader.end() && (*it).second == "close")
				return (1);

			time(&get_time);
			if (get_time - client.timeStart > TIME_KEEP_ALIVE)
				return (1);

			return (0);
		}
//ADD__________________________________________________________________________
		std::string	get_status_code(const int& code)
		{
			switch (code)
			{
				case 200:
					return ("200 OK");
				case 201:
					return ("201 Created");
				case 204:
					return ("204 No Content");
				case 301:
					return ("301 Moved Permanently");
				case 400:
					return ("400 Bad Request");
				case 404:
					return ("404 Not Found");
				case 405:
					return ("405 Method Not Allowed");
				case 413:
					return ("413 Payload Too Large");
				case 500:
					return ("500 Internal Server Error");
				case 501:
					return ("501 Not Implemented");
				default:
					return ("Not found");
			}
		}
//DEBUG________________________________________________________________________
		void		makeServer(Server& serv) { server.push_back(serv); }

	public:
		std::vector<Server>					server;
		std::vector<Client>					client;
		std::vector<int>					listenSocket;
};

//# include "readConfigFile.cpp"

#endif
