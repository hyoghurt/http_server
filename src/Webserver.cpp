#include "Webserver.hpp"

Webserver::Webserver() {}
Webserver::Webserver(const Webserver& oth) { *this = oth; }
Webserver::~Webserver()
{
	std::vector<int>::iterator		it;
	std::vector<Client>::iterator	it_c;

	for (it = listenSocket.begin(); it != listenSocket.end(); ++it)
		close (*it);
	for (it_c = client.begin(); it_c != client.end(); ++it_c)
		close ((*it_c).getSocket());
}

Webserver&	Webserver::operator= (const Webserver& oth)
{
	this->server = oth.server;
	this->client = oth.client;
	this->listenSocket = oth.listenSocket;
	return *this;
}

//CREATE_LISTEN_SOCKET_________________________________________________________
int			Webserver::createSocketListen()
{
	std::vector<Server>::iterator	it;
	std::string						ip;
	std::string						port;
	int								l_socket;

	for (it = server.begin(); it != server.end(); ++it)
	{
		ip = (*it).getIpAddress();
		port = (*it).getPort();

		if (-1 != checkIpAddressAndPort(ip, port, it))
		{
			l_socket = createSListen(ip, port);
			if (-1 == l_socket)
				return (-1);
			listenSocket.push_back(l_socket);
		}
	}
	return (0);
}

int		Webserver::checkIpAddressAndPort(const std::string& ip,
			const std::string& port, std::vector<Server>::iterator it_end)
{
	std::vector<Server>::iterator	it;

	for (it = server.begin(); it != it_end; ++it)
		if ((*it).getIpAddress() == ip and (*it).getPort() == port)
			return (-1);
	return (0);
}

int		Webserver::createSListen(const std::string& ip, const std::string& port)
{
	//создание сокета__________________________________________________
	int	l_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (l_socket == -1)
		return (print_error("socket"));

	//перевод сокета в неблокирующий режим_____________________________
	int	flags = fcntl(l_socket, F_GETFL);
	if (-1 == fcntl(l_socket, F_SETFL, flags | O_NONBLOCK))
		return (print_error("fcntl"));

	//установили флаг, для предотвращения залипаниz TCP порта__________
	int	opt = 1;
	if (-1 == setsockopt(l_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
		return (print_error("setsockopt"));

	//сопостовление созданному сокету конкретного адреса_______________
	struct sockaddr_in  addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port.c_str()));
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	if (-1 == bind(l_socket,(struct sockaddr*)&addr, sizeof(addr)))
		return (print_error("bind"));

	//переводим сокет в слушаюший режим________________________________
	if (-1 == listen(l_socket, 128))
		return (print_error("listen"));

	//печать на консоль информацию______________________________________
	std::cout << PINK << "\n__START SERVER: SOCKET:" << l_socket;
	std::cout << " " << ip << ":" << port << "    " << NO_C << '\n';
	return (l_socket);
}
//START_WEBSERVER______________________________________________________________
int		Webserver::start()
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
void	Webserver::createFdSet(int &max_d, fd_set &readfds, fd_set &writefds)
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
void	Webserver::addNewClient(fd_set &readfds)
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
				client.push_back(Client(scl, acl));
				client.back().debugInfConnect(*ils);
			}
		}
	}
}
//PROCESSING_CLIENT____________________________________________________________
void	Webserver::processingClient(fd_set &readfds, fd_set &writefds)
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
			(*it_client).debugInfClose("close socket (read = -1)");
			close(socket_client);
			it_client = client.erase(it_client);
			continue;
		}

		//данные_для_отправки__________________________________________
		if (FD_ISSET(socket_client, &writefds))
			bytes = (*it_client).writeSocket();

		//закрытие_клиента_____________________________________________
		if (bytes < 0 || checkCloseClient(*it_client))
		{
			if (bytes < 0)
				(*it_client).debugInfClose("close socket (write = -1)");
			else
				(*it_client).debugInfClose("close socket (time > "
						+ std::to_string(TIME_KEEP_ALIVE) + ")");
			close(socket_client);
			it_client = client.erase(it_client);
			continue;
		}
		++it_client;
	}
}
//READ_SOCKET__________________________________________________________________
int		Webserver::readSocket(Client& client)
{
	char		buf[BUF_SIZE];


	//чтение_данных_от_клиента_________________________________________
	int	bytes = recv(client.getSocket(), buf, BUF_SIZE, 0);

	if (bytes > 0)
	{
		client.setTimeStart();
		//client.request.append(buf, bytes);
		client.setRequestAppend(buf, bytes);

		if (client.getReadByte() == 0 && client.getChunked() == 0)
			client.readEraseHeaderRequest();
		if (client.getChunked() > 0)
			client.readExecChunk();

		//получили_данные_полностью____________________________________
		if (client.readSocketCheckEndRead())
			createResponse(client, 0);
	}
	return (bytes);
}
//RESPONSE_____________________________________________________________________
void	Webserver::createResponse(Client& client, const int& max_header)
{
	int				status_code;
	std::string		path_file;

	//выполнение_запроса_______________________________________________
	if (max_header == 413)
		status_code = 413;
	else
		status_code = processingResponse(client);

	//формирование_итога_для_отправки__________________________________
	client.setResponseHeaderStatus(status_code);
	if (200 != status_code)
	{
		path_file = client.getPathErrorPage(status_code);
		if (!path_file.empty())
			client.getOpenFile(path_file);
	}
	client.responseTotal();
	client.upFWrite();
}
//RESPONSE_____________________________________________________________________
int		Webserver::processingResponse(Client& client)
{
	std::map<std::string, std::string>::iterator	it;

	//привязка_к_config_server_________________________________________
	if (findServer(client))
		return (400);

	//вытащить_переменные_для_cgi______________________________________
	client.findQueryStringPathInfo();

	//привязка_к_config_location_______________________________________
	if (client.locationFind())
		return (400);

	//редирект_________________________________________________________
	if (client.check301())
		return (client.getLocation()->getReturnCode());

	//проверка_размера_тела_запроса____________________________________
	if (client.check413())
		return (413);

	//проверка_метода_(GET,POST,DELETE)________________________________
	if (client.check501())
		return (501);

	//проверка_метода_допустимых_в_config______________________________
	if (client.check405())
		return (405);

	//выполнение_метода_DELETE_________________________________________
	if (client.getRequestMethod() == "DELETE")
		return (client.runDelete());

	//выполнение_CGI___________________________________________________
	if (!client.getLocation()->getCgiPass().empty())
		return (client.runCgi());

	//выполнение_метода_GET____________________________________________
	if (client.getRequestMethod() == "GET")
		return (client.runGet());

	//выполнение_метода_POST___________________________________________
	return (client.runPost());
}
//CONFIG_SERVER________________________________________________________________
int		Webserver::findServer(Client& client)
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

	if (host == "localhost"){
		host = "127.0.0.1";
	}

	for (it = server.begin(); it != server.end(); ++it)
		if ((*it).getIpAddress() == host && (*it).getPort() == port)
		{
			client.setServer(&(*it));
			return (0);
		}

	for (it = server.begin(); it != server.end(); ++it)
		if ((*it).getServerName() == host)
			client.setServer(&(*it));

	if (client.getServer() == nullptr)
		return (1);
	return (0);
}
//CHECK_CLOSE_CLIENT___________________________________________________________
bool	Webserver::checkCloseClient(Client& client)
{
	time_t			get_time;

	if (client.getFWrite())
		return (false);
	if (client.checkResponseHeaderConnectionClose())
		return (true);
	if (time(&get_time) - client.getTimeStart() > TIME_KEEP_ALIVE)
		return (true);
	return (false);
}
