# include "Server.hpp"

Server::Server()
{
	ipAddress = "";
	port = "";
	serverName = "";
	clientMaxBodySize = -1;
}

Server::Server(const Server &oth)
{ *this = oth; }

Server::~Server()
{}

Server&				Server::operator= (const Server &oth)
{
	this->ipAddress = oth.ipAddress;
	this->port = oth.port;
	this->serverName = oth.serverName;
	this->clientMaxBodySize = oth.clientMaxBodySize;
	this->location = oth.location;
	this->errorPage = oth.errorPage;
	return *this;
}

Location			*Server::findLocationRule(const std::string& rule)
{
	std::vector<Location>::iterator	it;

	for (it = location.begin(); it != location.end(); ++it)
		if ((*it).getRule() == rule)
			return (&(*it));
	return (nullptr);
}

void				Server::clear()
{
	ipAddress = "";
	port = "";
	serverName = "";
	clientMaxBodySize = -1;
	errorPage.clear();
	location.clear();
}

const std::string&	Server::getIpAddress() const
{ return this->ipAddress; }

const std::string&	Server::getPort() const
{ return this->port; }

const std::string&	Server::getServerName() const
{ return this->serverName; }

const int&			Server::getClientMaxBodySize() const
{ return this->clientMaxBodySize; }

void				Server::setIpAddress(const std::string& str)
{ this->ipAddress = str; }

void				Server::setPort(const int& i)
{ this->port = i; }

void				Server::setServerName(const std::string& str)
{ this->serverName = str;  }

void				Server::setClientMaxBodySize(const int& size)
{ this->clientMaxBodySize = size; }

void				Server::setErrorPage(const std::map<int, std::string>& err)
{ this->errorPage = err; }

void				Server::setLocation(const std::vector<Location>& loc)
{ this->location = loc; }