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

Location*			Server::findLocationRule(const std::string& rule)
{
	std::vector<Location>::iterator	it;

	for (it = location.begin(); it != location.end(); ++it)
		if ((*it).getRule() == rule)
			return (&(*it));
	return (nullptr);
}

bool				Server::checkLocationEmpty()
{ return (location.empty()); }

void				Server::addClientMaxBodySize(const int& i)
{
	std::vector<Location>::iterator		lc;

	for (lc = location.begin(); lc != location.end(); ++lc)
		if (clientMaxBodySize == i)
			setClientMaxBodySize(i);
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

const std::string	Server::getPathErrorPage(const int& code)
{
	std::map<int, std::string>::iterator	it;

	it = errorPage.find(code);
	if (it != errorPage.end())
		return (*it).second;
	return ("");
}

void				Server::setIpAddress(const std::string& str)
{ this->ipAddress = str; }

void				Server::setPort(const std::string& i)
{ this->port = i; }

void				Server::setServerName(const std::string& str)
{ this->serverName = str;  }

void				Server::setClientMaxBodySize(const int& size)
{ this->clientMaxBodySize = size; }

void				Server::setErrorPage(const int& i, const std::string& str)
{ this->errorPage[i] = str; }

void				Server::setLocation(const Location& loc)
{ this->location.push_back(loc); }
