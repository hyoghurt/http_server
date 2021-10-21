#ifndef SERVER_HPP
# define SERVER_HPP

# include "add_func.hpp"
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
		Server()
		{
			ipAddress = "";
			port = "";
			serverName = "";
			clientMaxBodySize = -1;
		}

		Server(const Server &oth)	{ *this = oth; }
		~Server() {}

		Server&	operator= (const Server &oth)
		{
			this->ipAddress = oth.ipAddress;
			this->port = oth.port;
			this->serverName = oth.serverName;
			this->clientMaxBodySize = oth.clientMaxBodySize;
			this->location = oth.location;
			this->errorPage = oth.errorPage;
			return *this;
		}

		std::string					getIpAddress() const
		{ return this->ipAddress; }

		std::string					getPort() const
		{ return this->port; }

		std::string					getServerName() const
		{ return this->serverName; }

		int							getClietnMaxBodySize() const
		{ return this->clientMaxBodySize; }

		std::map<int, std::string>&	getErrorPage()
		{ return this->errorPage; }

		std::vector<Location>&		getLocation()
		{ return this->location; }

		void				setIpAddress(const std::string& str)
		{ this->ipAddress = str; }

		void				setPort(const int& port)
		{ this->port = port; }

		void				setServerName(const std::string& str)
		{ this->serverName = str;  }

		void				setClietnMaxBodySize(const int& size)
		{ this->clientMaxBodySize = size; }

		void				setErrorPage(const std::map<int, std::string>& err)
		{ this->errorPage = err; }

		void				setLocation(const std::vector<Location>& loc)
		{ this->location = loc; }

		Location*			findLocationRule(const std::string& rule)
		{
			std::vector<Location>::iterator	it;

			for (it = location.begin(); it != location.end(); ++it)
				if ((*it).getRule() == rule)
					return (&(*it));
			return (nullptr);
		}

		void				clear()
		{
			ipAddress = "";
			port = "";
			serverName = "";
			clientMaxBodySize = -1;
			errorPage.clear();
			location.clear();
		}

	public:
		std::string							ipAddress;
		std::string							port;
		std::string							serverName;
		int									clientMaxBodySize;
		std::map<int, std::string>			errorPage;
		std::vector<Location>				location;
};

#endif
