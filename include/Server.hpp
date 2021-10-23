#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include <map>

class	Server
{
	public:
		Server();
		Server(const Server &oth);
		~Server();

		Server&				operator= (const Server& oth);
		Location*			findLocationRule(const std::string& rule);
		bool				checkLocationEmpty();
		void				addClientMaxBodySize(const int& i);
		void				clear();

		const std::string&	getIpAddress() const;
		const std::string&	getPort() const;
		const std::string&	getServerName() const;
		const int&			getClientMaxBodySize() const;
		const std::string	getPathErrorPage(const int& code);

		void				setIpAddress(const std::string& str);
		void				setPort(const std::string& port);
		void				setServerName(const std::string& str);
		void				setClientMaxBodySize(const int& size);
		void				setErrorPage(const int& i, const std::string& str);
		void				setLocation(const Location& loc);

		void				multiServerName()
		{
			if (serverName == "")
				setServerName(ipAddress + ":" + port);
		}


	private:
		std::string							ipAddress;
		std::string							port;
		std::string							serverName;
		int									clientMaxBodySize;
		std::map<int, std::string>			errorPage;
		std::vector<Location>				location;
};

#endif
