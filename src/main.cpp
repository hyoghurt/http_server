# define DEBUG

#include "Webserver.hpp"
#include "Location.hpp"
#include <cstdlib>

int	main(int argc, char **argv)
{
	Webserver				webserver;

	//читаем конфиг файл______________________________________________
#ifdef DEBUG
	/*
	Server		server1;

	server1.ipAddress = "127.0.0.1";
	server1.port = "9000";
	server1.serverName = "test.ru";
	server1.clientMaxBodySize = 10000;
	server1.errorPage[400] = "www/400.html";
	server1.errorPage[404] = "www/404.html";

	server1.location["/"].root = "www";
	server1.location["/"].autoindex = true;
	server1.location["/"].index = "index.html";
	server1.location["/"].accessMethods.push_back("GET");
	server1.location["/"].accessMethods.push_back("POST");

	server1.location["/redir"].return_code = 301;
	server1.location["/redir"].return_location = "https://www.patatap.com";

	server1.location[".py"].root = "www";
	server1.location[".py"].cgiPass = webserver.absolutePathOfExec("python3");
	server1.location[".py"].accessMethods.clear();
	server1.location[".py"].accessMethods.push_back("GET");
	server1.location[".py"].accessMethods.push_back("POST");

	server1.location["/uploads"].root = "www/uploads";
	server1.location["/uploads"].autoindex = true;
	server1.location["/uploads"].uploadPass = true;
	server1.location["/uploads"].accessMethods.clear();
	server1.location["/uploads"].accessMethods.push_back("GET");
	server1.location["/uploads"].accessMethods.push_back("DELETE");

	server1.location["/dowloads"].root = "www/dowloads";
	server1.location["/dowloads"].autoindex = true;
	server1.location["/dowloads"].dowloadPass = true;
	server1.location["/dowloads"].accessMethods.clear();
	server1.location["/dowloads"].accessMethods.push_back("GET");
	server1.location["/dowloads"].accessMethods.push_back("POST");
	server1.location["/dowloads"].accessMethods.push_back("DELETE");
	*/

	Server		server1;

	server1.ipAddress = "127.0.0.1";
	server1.port = "9000";
	server1.serverName = "test_1.ru";
	server1.clientMaxBodySize = 10000;
	server1.errorPage[400] = "www/400.html";
	server1.errorPage[404] = "www/404.html";

	server1.location["/"].root = "www";
	server1.location["/"].autoindex = true;
	server1.location["/"].accessMethods.push_back("GET");
	server1.location["/"].accessMethods.push_back("POST");

	webserver.makeServer(server1);

	server1.ipAddress = "127.0.0.1";
	server1.port = "9000";
	server1.serverName = "test_2.ru";
	server1.clientMaxBodySize = 10000;
	server1.errorPage[400] = "www/400.html";
	server1.errorPage[404] = "www/404.html";

	server1.location["/"].root = "www/no_index";
	server1.location["/"].autoindex = true;
	server1.location["/"].accessMethods.push_back("GET");
	server1.location["/"].accessMethods.push_back("POST");

	webserver.makeServer(server1);

#endif

	if (-1 == webserver.createSocketListen())
		return (1);

	if (-1 == webserver.start())
		return (1);

	return (0);
}
