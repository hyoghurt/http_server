#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

//# define TIME_KEEP_ALIVE 

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

# include <cmath> //pow

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
 *
 *
 *
 * исправить тест с размером тела 100000000 и отправки его в cgi
 * исправить код ответа при создании файла
 * подисать cgi окружение
 * написать signal для выхода из программы
 * запустить tester
 * запустить siege
 *
 * раширение .cgi прочесть файл и найти интерпритатор
 * корректировка парсера полученного хедера (пробелы и перенос строки)
 *
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
		//Webserver() : log_file("log.log")
		Webserver()
		{
			find_interpreter();
		}
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
			this->interpreter = oth.interpreter;
			return *this;
		}
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
			if (-1 == listen(socketListen, 5))
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
				FD_SET((*icl).getSocket(), &readfds);
				FD_SET((*icl).getSocket(), &writefds);
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

				//данные_для_чтения____________________________________________
				if (FD_ISSET(socket_client, &readfds))
					bytes = readSocket(*it_client);

				//данные_для_отправки__________________________________________
				//if ((*it_client).flag == 1 || FD_ISSET(socket_client, &writefds))
				if ((*it_client).flag)
					bytes = writeSocket(*it_client);


				//закрытие_клиента_____________________________________________
				//if (bytes == -1 || checkCloseClient(*it_client))
				if (checkCloseClient(*it_client))
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
		void	erase_header_request(Client& client)
		{
			std::map<std::string, std::string>::iterator	it;
			size_t											found;

			found = client.request.find("\r\n\r\n");

			if (found != std::string::npos)
			{
				std::cout << "----------------------------\n";
				std::cout << client.request << std::endl;
				std::cout << "----------------------------\n";
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
		int		readSocket(Client& client)
		{
			int				f, f2;
			std::string		len;
			size_t			found, found2;
			std::map<std::string, std::string>::iterator	it;
			int				max_header(0);

			//чтение_данных_от_клиента_________________________________________
			int	bytes = recv(client.getSocket(), client.buf, BUF_SIZE - 1, 0);

			if (bytes > 0)
			{
				//client.debug_info("get bytes: " + std::to_string(bytes));
				client.setTimeStart();
				client.request.append(client.buf, bytes);

				if (client.readByte == 0)
					erase_header_request(client);
				while (client.chunked > 0)
				{
					if (client.chunked == 1)
					{
						found = client.request.find("\r\n");
						if (found != std::string::npos)
						{
							client.readByte = convert_str_to_base16(client.request.substr(0, found));

							if (client.readByte > 0)
							{
								client.chunked = 2;
								client.request.erase(0, found + 2);
							}
							else
							{
								client.chunked = 0;
								client.readByte = client.request.size();
							}
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
							client.chunked = 1;
						}
						else
						{
							break ;
						}
					}
				}

				//получили_данные_полностью____________________________________
				if (client.chunked == 0 && client.request.size() == client.readByte)
				{
					client.readByte = 0;

					if (client.json_request.find("Content-Length") != client.json_request.end())
						client.json_request["body"] = client.request;

					std::cout << "----------------------------------------" << std::endl;
					std::cout << "BODY SIZE = " << client.json_request["body"].size() << std::endl;
					std::cout << "-----------------------------------------" << std::endl;

					create_response(client, max_header);
					//закрытие_сокета_чтения___________________________________
					//shutdown(client.socket, 0);
				}
			}
			return (bytes);
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
			std::string		tmp;
			int				bytes;

			std::cout << "---------------------------------\n";
			std::cout << client.response.size() << std::endl;
			std::cout << "---------------------------------\n";

			/*
			if (client.response.size() > 1000)
			{
				tmp = client.response.substr(0, 1000);
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

			bytes = send(client.socket, client.response.c_str(),
					client.response.size(), 0);

			client.setTimeStart();
			if (bytes > 0)
			{
				client.response.erase(0, bytes);
				client.debug_info("write bytes: " + std::to_string(bytes)
					+ " ost bytes:" + std::to_string(client.response.size()));

				if (client.response.empty())
				{
					client.flag = 0;
				}
			}

			client.debug_info("write bytes response: " + std::to_string(bytes)
					+ " ost bytes:" + std::to_string(client.response.size()));

				/*
			if (check_chunk_header_response(client))
			{
				std::cout << "--HEADER-------------------------\n";
				std::cout << client.header.size() << std::endl;
				std::cout << "---------------------------------\n";
				std::cout << "--BODY---------------------------\n";
				std::cout << client.body.size() << std::endl;
				std::cout << "---------------------------------\n";

				if (!client.header.empty())
				{
					bytes = send(client.socket, client.header.c_str(), client.header.size(), 0);
					if (bytes > 0)
						client.header.erase(0, bytes);

					client.debug_info("write bytes header: " + std::to_string(bytes)
						+ " ost bytes:" + std::to_string(client.header.size()));
				}
				else
				{
					bytes = send(client.socket, client.body.c_str(), client.body.size(), 0);
					if (bytes > 0)
						client.body.erase(0, bytes);

					client.debug_info("write bytes body: " + std::to_string(bytes)
						+ " ost bytes:" + std::to_string(client.body.size()));
				}
			}
			else
			{
				bytes = send(client.socket, client.response.c_str(),
						client.response.size(), 0);

				if (bytes > 0)
					client.response.erase(0, bytes);

				if (client.response.empty())
				{
					client.flag = 0;
				}
				client.debug_info("write bytes response: " + std::to_string(bytes)
						+ " ost bytes:" + std::to_string(client.response.size()));
			}
				*/

			client.setTimeStart();
			return (bytes);

			/*
			if (client.response.size() > 16384)
			{
				tmp = client.response.substr(0, 16384);
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
			*/
			/*
			//отправка_данных_клиенту__________________________________________
			int	bytes = send(client.socket, client.response.c_str(),
					client.response.size(), 0);

			client.setTimeStart();
			if (bytes > 0)
			{

				//client.setTimeStart();
				client.response.erase(0, bytes);
				client.debug_info("write bytes: " + std::to_string(bytes)
					+ " ost bytes:" + std::to_string(client.response.size()));

				if (client.response.empty())
				{
					client.flag = 0;
				}
				//закрытие_сокета_отправкиi____________________________________
				if (client.response.empty())
					shutdown(client.socket, 1);
			}
			*/
		}
//RESPONSE_____________________________________________________________________
		void	create_response(Client& client, const int& max_header)
		{
			print_debug("F create response");

			int				status_code;
			std::string		path_file;

			client.responseHeader.clear();
			client.body.clear();

			//client.debug_show_map(client.json_request);

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

			std::cout << client.header << std::endl;
			//std::cout << client.response << std::endl;
			//std::string		check(client.response, 400);
			//std::cout << check << std::endl;

			//отправка_данных_клиенту__________________________________________
			client.flag = 1;
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

			//выполнение_загрузки______________________________________________
			if (client.location->uploadPass)
				return (client.get_run());

			//выполнение_скачивания____________________________________________
			if (client.location->dowloadPass)
			{
				if (client.json_request["method"] == "GET")
					return (client.get_run());
				return (client.post_run());
			}

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
							client.responseHeader["Location"] = client.json_request["request_target"] + "/";
							return (301);
						}

						d = check_dir_or_file(client.path_file + client.location->index);
						/*
						if (d != -1 && !client.location->autoindex)
							client.path_file.append(client.location->index);
							*/
						if (d == -1)
						{
							if (!client.location->autoindex)
								client.path_file.append(client.location->index);
						}
						if (d == 0)
							client.path_file.append(client.location->index);
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
//FIND_INTERPRETER_____________________________________________________________
		void	find_interpreter()
		{
			std::map<std::string, std::string>				tmp;
			std::map<std::string, std::string>::iterator	it;

			tmp[".py"]		= "python3";
			tmp[".php"]		= "php";
			tmp[".perl"]	= "perl";
			tmp[".rb"]		= "ruby";
			tmp[".rbw"]		= "ruby";
			tmp[".sh"]		= "sh";

			for (it = tmp.begin(); it != tmp.end(); ++it)
				tmp[(*it).first] = absolutePathOfExec((*it).second);

			for (it = tmp.begin(); it != tmp.end(); ++it)
				if (!(*it).second.empty())
					interpreter.insert(*it);

			std::cout <<  YELLOW << "\nCGI: interpreter     found: " << RESET;
			for (it = interpreter.begin(); it != interpreter.end(); ++it)
				std::cout << " | " << (*it).first;
			std::cout << '\n';

			std::cout << YELLOW << "CGI: interpreter not found: " << RESET;
			for (it = tmp.begin(); it != tmp.end(); ++it)
				if ((*it).second.empty())
					std::cout << " | " << (*it).first;
			std::cout << '\n';
		}
//FIND_INTERPRETER_____________________________________________________________
		std::string		absolutePathOfExec(const std::string& execName)
		{
			std::string		absolutePath;
			std::string		token;
			std::string		path = getenv("PATH");
			size_t			pos = 0;
			struct stat		st;
		
			if (path.empty())
				return ("");
		
			while ((pos = path.find(':')) != std::string::npos)
			{
				token = path.substr(0, pos);
				absolutePath = (token.append("/") + execName);
		
				if (stat(absolutePath.c_str(), &st) == 0)
					return absolutePath;
		
				path.erase(0, pos + 1);
			}
			return ("");
		}
//CHECK_CLOSE_CLIENT___________________________________________________________
		int		checkCloseClient(Client& client)
		{
			time_t		get_time;

			if (!client.response.empty())
				return (0);
			/*
			if (check_chunk_header_response(client))
			{
				std::cout << RED << "CLOSE check chunk" << RESET << std::endl;
				if (!client.body.empty() || !client.header.empty())
				{
					std::cout << RED << "empty header and body" << RESET << std::endl;
					return (0);
				}
			}
			*/

			if (client.responseHeader["Connection"] == "close")
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
		std::map<std::string, std::string>	interpreter;
		//std::ofstream						log_file;
};

#endif
