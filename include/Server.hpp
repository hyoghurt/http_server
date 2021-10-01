#ifndef SERVER_HPP
# define SERVER_HPP

# include "add_funct.hpp"
# include "def_color.hpp"
# include "Location.hpp"

# include <iostream> //std
//# include <sys/socket.h>
//# include <sys/types.h>
//# include <netinet/in.h>
# include <arpa/inet.h> //inet_addr, sockaddr_in
# include <unistd.h> //close
# include <fcntl.h> //fntnl
# include <map>

class	Server
{
	public:
		Server()					{}
		Server(const Server &oth)	{ *this = oth; }
		~Server() {}

		Server&	operator= (const Server &oth)
		{
			this->ipAddress = oth.ipAddress;
			this->port = oth.port;
			this->serverName = oth.serverName;
			this->location = oth.location;
			this->errorPage = oth.errorPage;
			return *this;
		}

		std::string			getIpAddress(void) const
		{ return this->ipAddress; }

		std::string			getPort(void) const
		{ return this->port; }

		void				setIpAddress(const std::string& str)
		{ this->ipAddress = str; }

		void				setPort(const int& port)
		{ this->port = port; }

		Server(const std::string& ipAddress, const std::string& port,
				const std::string& serverName,
				const std::map<int, std::string> errorPage,
				const std::map<std::string, Location> location)
		{
			this->ipAddress = ipAddress;
			this->port = port;
			this->serverName = serverName;
			this->errorPage = errorPage;
			this->location = location;
		}

	public:
		std::string							ipAddress;
		std::string							port;
		std::string							serverName;
		std::map<int, std::string>			errorPage;
		std::map<std::string, Location>		location;
};

#endif
