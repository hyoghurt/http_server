# define TESTER

#include "Webserver.hpp"
#include "Location.hpp"
#include <cstdlib>
#include <sstream>

int	main(int argc, char **argv)
{
	Webserver				webserver;

	//читаем конфиг файл______________________________________________
#ifdef DEBUG
	std::cout << "DEBUG" << std::endl;
	Server		server1;
	Location	loc;

	server1.ipAddress = "127.0.0.1";
	server1.port = "9000";
	server1.serverName = "test.ru";
	server1.clientMaxBodySize = 10000;
	server1.errorPage[400] = "www/400.html";
	server1.errorPage[404] = "www/404.html";

	loc.clear();
	loc.rule = "/";
	loc.root = "www";
	loc.autoindex = true;
	loc.index = "index.html";
	loc.accessMethods.push_back("GET");
	loc.accessMethods.push_back("POST");
	server1.location.push_back(loc);

	loc.clear();
	loc.rule = "/redir";
	loc.return_code = 301;
	loc.return_location = "https://www.patatap.com";
	server1.location.push_back(loc);

	loc.clear();
	loc.rule = ".py";
	loc.cgiPass = webserver.absolutePathOfExec("python3");
	loc.accessMethods.clear();
	loc.accessMethods.push_back("GET");
	loc.accessMethods.push_back("POST");
	server1.location.push_back(loc);

	loc.clear();
	loc.rule = "/uploads";
	loc.root = "www/uploads";
	loc.autoindex = true;
	loc.uploadPass = true;
	loc.accessMethods.clear();
	loc.accessMethods.push_back("GET");
	loc.accessMethods.push_back("DELETE");
	server1.location.push_back(loc);

	loc.clear();
	loc.rule = "/dowloads";
	loc.root = "www/dowloads";
	loc.autoindex = true;
	loc.dowloadPass = true;
	loc.accessMethods.clear();
	loc.accessMethods.push_back("GET");
	loc.accessMethods.push_back("POST");
	loc.accessMethods.push_back("DELETE");
	server1.location.push_back(loc);
#endif

#ifdef TESTER
	std::cout << "TESTER" << std::endl;
	Server		server1;
	Location	loc;

	server1.ipAddress = "127.0.0.1";
	server1.port = "9080";
	server1.serverName = "test_1.ru";
	server1.errorPage[400] = "www/400.html";
	server1.errorPage[404] = "www/404.html";

	loc.rule = "/";
	loc.root = "YoupiBanane";
	//loc.root = ".";
	loc.index = "youpi.bad_extension";
	loc.accessMethods.clear();
	loc.accessMethods.push_back("GET");
	server1.location.push_back(loc);

	loc.clear();
	loc.rule = "/put_test";
	loc.root = ".";
	loc.accessMethods.clear();
	loc.accessMethods.push_back("PUT");
	server1.location.push_back(loc);

	loc.clear();
	loc.rule = ".bla";
	//loc.cgiPass = webserver.absolutePathOfExec("cgi_test");
	loc.cgiPass = "/Users/hyoghurt/ft_webserver/cgi_tester";
	loc.accessMethods.clear();
	loc.accessMethods.push_back("GET");
	loc.accessMethods.push_back("POST");
	server1.location.push_back(loc);

	loc.clear();
	loc.rule = "/post_body";
	loc.root = ".";
	loc.clientMaxBodySize = 100;
	loc.accessMethods.clear();
	loc.accessMethods.push_back("POST");
	server1.location.push_back(loc);

	loc.clear();
	loc.rule = "/directory";
	loc.root = "YoupiBanane";
	loc.index = "youpi.bad_extension";
	loc.accessMethods.clear();
	loc.accessMethods.push_back("GET");
	server1.location.push_back(loc);

#endif
	webserver.makeServer(server1);

	if (-1 == webserver.createSocketListen())
		return (1);

	if (-1 == webserver.start())
		return (1);

	return (0);
}
