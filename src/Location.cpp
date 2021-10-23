#include "Location.hpp"

Location::Location ()
{
	rule = "";
	root = "";
	autoindex = false;
	index = "index.html";
	returnCode = 0;
	returnLocation = "";
	cgiPass = "";
	clientMaxBodySize = -1;
}

Location::Location (const Location& oth)
{ *this = oth; }

Location::~Location ()
{}

Location&	Location::operator= (const Location& oth)
{
	this->rule = oth.rule;
	this->root = oth.root;
	this->autoindex = oth.autoindex;
	this->index = oth.index;
	this->accessMethods = oth.accessMethods;
	this->returnCode = oth.returnCode;
	this->returnLocation = oth.returnLocation;
	this->cgiPass = oth.cgiPass;
	this->clientMaxBodySize = oth.clientMaxBodySize;
	return *this;
}

void	Location::clear()
{
	rule = "";
	root = "";
	autoindex = false;
	index = "index.html";
	accessMethods.clear();
	returnCode = 0;
	returnLocation = "";
	cgiPass = "";
	clientMaxBodySize = -1;
}

bool					Location::checkAccessMethod(const std::string& method)
{
	std::vector<std::string>::iterator	it;

	for (it = accessMethods.begin(); it != accessMethods.end(); ++it)
		if (*it == method)
			return (true);
	return (false);
}

const std::string&		Location::getRule() const
{ return this->rule; }

const std::string&		Location::getRoot() const
{ return this->root; }

const bool&				Location::getAutoindex() const
{ return this->autoindex; }

const std::string&		Location::getIndex() const
{ return this->index; }

const int&				Location::getReturnCode() const
{ return this->returnCode; }

const std::string&		Location::getReturnLocation() const
{ return this->returnLocation; }

const std::string&		Location::getCgiPass() const
{ return this->cgiPass; }

const int&				Location::getClientMaxBodySize() const
{ return this->clientMaxBodySize; }

void			Location::setRule(const std::string& str)
{ this->rule = str; }

void			Location::setRoot(const std::string& str)
{ this->root = str; }

void			Location::setAutoindex(const bool& b)
{ this->autoindex = b; }

void			Location::setIndex(const std::string& str)
{ this->index = str; }

void			Location::setAccessMethods(const std::string& str)
{ this->accessMethods.push_back(str); }

void			Location::setReturn(const int& i, const std::string& str)
{
	this->returnCode = i;
	this->returnLocation = str;
}

void			Location::setReturnCode(const int& i)
{ this->returnCode = i; }

void			Location::setReturnLocation(const std::string& str)
{ this->returnLocation = str; }

void			Location::setCgiPass(const std::string& str)
{ this->cgiPass = str; }

void			Location::setClientMaxBodySize(const int& i)
{ this->clientMaxBodySize = i; }

