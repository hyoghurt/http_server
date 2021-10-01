# define DEBUG

#include "Webserver.hpp"
#include "add_funct.hpp"
#include "Location.hpp"
#include <stdlib.h>

int	main(int argc, char **argv)
{
	Webserver				webserver;

#ifdef DEBUG
	std::string							root;
	bool								autoindex;
	std::string							index;
	std::vector<std::string>			accessMethods;

	std::string							host;
	std::string							port;
	std::string							server_name;
	std::map<int, std::string>			error_page;
	std::map<std::string, Location>		location;

	root = "www";
	autoindex = false;
	index = "index.html";
	//index.push_back("index.php");
	accessMethods.push_back("GET");
	accessMethods.push_back("POST");

	Location		loc(root, autoindex, index, accessMethods);

	location["/"] = loc;

	host = "127.0.0.1";
	port = "8000";
	server_name = "test.ru";
	error_page[400] = "www/400.html";

	Server			serv(host, port, server_name, error_page, location);

	webserver.makeServer(serv);
#endif

	/*
	std::string			a;
	int					in;
	a = "8000";
	in = atoi(a.c_str());
	*/

	//читаем конфиг файл______________________________________________

	if (-1 == webserver.createSocketListen())
		return (1);

	if (-1 == webserver.start())
		return (1);

	return (0);
}
