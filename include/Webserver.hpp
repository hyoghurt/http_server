#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

//# define TIME_KEEP_ALIVE 

# include <iostream>
# include <vector>
# include <fstream> //open, ifstream
# include <stdlib.h> //atoi
# include <dirent.h> //opendir
# include <sys/stat.h> //stat or <sys/types.h> <unistd.h>

# include "Server.hpp"
# include "Client.hpp"

/*
 * если не .py .sh то ошибка в конфиг файле
 *
 *
 *
 *
 * location .py
 *		root				cgi-bin
 *		index				index.py
 *		autoindex			false
 *		return 302			127.0.0.1:6000
 *		access_methods		GET, POST
 *
 * location /dowloads
 *		root				www/dowloads
 *		index				index.html
 *		autoindex			true
 *		access_methods		GET
 *		uplaad_pass			on
 *		cgi_pass			python3
 *
 *
 *
 *
 *
 *
 * +	check write and read -1 and 0
 * +	переписать автоиндекс
 * +	переписать нахождение локатион
 * +	content-type: png, html
 *
 * скачать то Content-Type: application/octet-stream
 * использовать server_name
 * написат redirect
 *
 * подисать cgi post
 * подисать cgi окружение
 * написать signal для выхода из программы
 * запустить tester
 * запустить siege
 *
 * раширение .cgi прочесть файл и найти интерпритатор
 * корректировка парсера полученного хедера (пробелы и перенос строки)
 *
 *
 * curl --resolve example.com:80:127.0.0.1 http://example.com/
 * curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit"
 * in the configuration try to setup the same port multiple times. it should not work
*/

class	Webserver
{
	public:

//CONSTRUCTOR__________________________________________________________________
		Webserver()						{ find_interpreter(); }
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

//START_WEBSERVER______________________________________________________________
		int		start()
		{
			fd_set		readfds;
			fd_set		writefds;
			int			max_d;
			int			num(0);

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

				//данные_для_чтения____________________________________________
				if (FD_ISSET(socket_client, &readfds))
					bytes = readSocket(*it_client);

				//данные_для_отправки__________________________________________
				if (FD_ISSET(socket_client, &writefds))
					bytes = writeSocket(*it_client);

				//закрытие_клиента_____________________________________________
				if (bytes == -1 || checkCloseClient(*it_client))
				{
					(*it_client).debug_info("close socket");

					close(socket_client);
					it_client = client.erase(it_client);
					continue;
				}
				bytes = 0;

				++it_client;
			}
		}
//READ_SOCKET__________________________________________________________________
		int		readSocket(Client& client)
		{
			int				f, f2;
			std::string		len;

			//чтение_данных_от_клиента_________________________________________
			int	bytes = recv(client.getSocket(), client.buf, BUF_SIZE - 1, 0);

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
						f = client.request.find("Content-Length:");
						if (f != std::string::npos)
						{
							f2 = client.request.find("\r\n", f);
							len = client.request.substr(f + 16, f2 - (f + 16));
							client.readByte = atoi(len.c_str());
						}
						client.readByte += client.request.find("\r\n\r\n") + 4;
					}
				}
				//получили_данные_полностью____________________________________
				if (client.request.size() == client.readByte)
				{
					std::cout << client.request << std::endl;

					client.readByte = 0;
					//создание_ответа__________________________________________
					create_response(client);
					//закрытие_сокета_чтения___________________________________
					//shutdown(client.socket, 0);
				}
			}
			return (bytes);
		}
//WRITE_SOCKET_________________________________________________________________
		int		writeSocket(Client& client)
		{
			//отправка_данных_клиенту__________________________________________
			int	bytes = send(client.socket, client.response.c_str(),
					client.response.size(), 0);

			if (bytes > 0)
			{
				client.setTimeStart();
				client.response.erase(0, bytes);
				client.debug_info("write bytes: " + std::to_string(bytes)
					+ " ost bytes:" + std::to_string(client.response.size()));

				//закрытие_сокета_отправкиi____________________________________
				/*
				if (client.response.empty())
					shutdown(client.socket, 1);
					*/
			}
			return (bytes);
		}
//RESPONSE_____________________________________________________________________
		void	create_response(Client& client)
		{
			print_debug("F create response");

			int				status_code;
			std::string		path_file;

			//парсер_запроса_клиента___________________________________________
			client.create_json_request();
			client.request.clear();

			//выполнение_запроса_______________________________________________
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

			//проверка_размера_тела_запроса____________________________________
			if (client.check_413())
				return (413);

			//вытащить_переменные_для_cgi______________________________________
			client.find_query_string_path_info();

			//привязка_к_config_location_______________________________________
			client.location = nullptr;
			find_location_filename_extension(client);
			if (client.location == nullptr)
			{
				find_location_directory(client);
				find_location_filename_extension(client);
			}
			if (client.location == 0)
				return (400);

			//проверка_метода_(GET,POST,DELETE)________________________________
			if (client.check_501())
				return (501);

			//проверка_метода_допустимых_в_config______________________________
			if (client.check_405())
				return (405);

			//выполнение_метода_DELETE_________________________________________
			if (client.json_request["method"] == "DELETE")
				return (client.deleteFile());

			//выполнение_CGI___________________________________________________
			for (it = interpreter.begin(); it != interpreter.end(); ++it)
			{
				if (client.path_file.find((*it).first) != std::string::npos)
				{
					client.interpreter = (*it).second;
					return (client.cgi_run());
				}
			}

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
				if ((*it).ipAddress == ip && (*it).port == port)
					return (&(*it));

			return (0);
		}
//CONFIG_LOCATION______________________________________________________________
		void	find_location_directory(Client &client)
		{
			std::map<std::string, Location>::iterator	it;
			std::string									request_target;
			int											found;

			request_target = client.path_file;
			print_debug("F find loacation for target: ["+request_target+"]");

			while (request_target.length() != 0)
			{
				it = client.server->location.find(request_target);
				if (it != client.server->location.end())
				{
					client.location = &(*it).second;
					add_root_and_index((*it).first, client);
					return ;
				}

				if (request_target == "/")
					return ;

				found = request_target.find_last_of("/");
				if (std::string::npos == found)
					return ;

				if (found == 0)
					found = 1;

				request_target.erase(found);
			}
		}
//CONFIG_LOCATION______________________________________________________________
		void	find_location_filename_extension(Client& client)
		{
			std::map<std::string, Location>::iterator	it;
			std::string									request_target;
			int											found;

			found = client.path_file.find_last_of('.');
			if (found != std::string::npos)
			{
				request_target = client.path_file.substr(found);
				it = client.server->location.find(request_target);

				if (it != client.server->location.end())
				{
					client.location = &(*it).second;
					add_root_and_index((*it).first, client);
				}
			}
		}
//CONFIG_LOCATION______________________________________________________________
		void	add_root_and_index(const std::string& role, Client& client)
		{
			print_debug("F add root and index (location: ["+role+"])");
			std::string	tmp_path_file = client.path_file;
			size_t		size;
			int			dir;

			if (role != "/" && role.find('.') == std::string::npos)
				tmp_path_file.erase(0, role.size());

			client.path_file = client.location->root + tmp_path_file;

			size = client.path_file.size();
			if (size == 0)
				return ;
			dir = check_dir_or_file(client.path_file);
			if (dir == 1)
			{
				if ('/' != client.path_file[size - 1])
					client.path_file += "/";
				if (check_dir_or_file(client.path_file + client.location->index) != 0)
					if (client.json_request["method"] == "GET" && client.location->autoindex)
						return ;
				client.path_file += client.location->index;
			}

			client.debug_info("client.path_file:" + client.path_file);
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
};

#endif
