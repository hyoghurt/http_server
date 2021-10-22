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
		void				clear();

		const std::string&	getIpAddress() const;
		const std::string&	getPort() const;
		const std::string&	getServerName() const;
		const int&			getClientMaxBodySize() const;

		void				setIpAddress(const std::string& str);
		void				setPort(const int& port);
		void				setServerName(const std::string& str);
		void				setClientMaxBodySize(const int& size);
		void				setErrorPage(const std::map<int, std::string>& err);
		void				setLocation(const std::vector<Location>& loc);


	public:
		std::string							ipAddress;
		std::string							port;
		std::string							serverName;
		int									clientMaxBodySize;
		std::map<int, std::string>			errorPage;
		std::vector<Location>				location;
};

#endif
