#ifndef WEBSERVER_HPP
# define WEBSERVER_HPP

# include <fcntl.h> //fntnl
//# include <csignal>

# include "Client.hpp"


class	Webserver
{
	public:
		Webserver();
		Webserver(const Webserver& oth);
		~Webserver();

		Webserver&	operator= (const Webserver& oth);

		void	setServer(const Server& serv);
		void	debug_show_conf();
		int		readConfigFile(const char* fileName);
		void	add_client_max_body_size();
		bool	checkСorrectField();
		bool	checkCorrectMethodName(std::vector <std::string> names);
		bool	checkCorrectIP(std::string ip);
		bool 	checkCorrectHost(std::string host);

		int		createSocketListen();
		int		checkIpAddressAndPort(const std::string& ip,
				const std::string& port, std::vector<Server>::iterator it_end);
		int		createSListen(const std::string& ip, const std::string& port);

		int		start();
		void	createFdSet(int &max_d, fd_set &readfds, fd_set &writefds);
		void	addNewClient(fd_set &readfds);
		void	processingClient(fd_set &readfds, fd_set &writefds);
		int		readSocket(Client& client);
		int		writeSocket(Client& client);

		void	createResponse(Client& client, const int& max_header);
		int		processingResponse(Client& client);
		int		findServer(Client& client);

		bool		checkCloseClient(Client& client);
		static void	signal_handler(int sig)
		{
			//this->~Webserver();
		}

	public:
		std::vector<Server>					server;
		std::vector<Client>					client;
		std::vector<int>					listenSocket;
};

#endif
