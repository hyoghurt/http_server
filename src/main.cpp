//# define DEBUG

#include "Webserver.hpp"
#include "Location.hpp"
#include <cstdlib>

int	main(int argc, char **argv)
{
	Webserver				webserver;

//#ifdef DEBUG
	std::string							root;
	bool								autoindex;
	std::string							index;
	std::vector<std::string>			accessMethods;

	std::string							host;
	std::string							port;
	std::string							server_name;
	int									body_size;
	std::map<int, std::string>			error_page;
	std::map<std::string, Location>		location;

	root = "www";
	autoindex = true;
	index = "index.html";
	accessMethods.clear();
	accessMethods.push_back("GET");
	accessMethods.push_back("POST");
	Location		loc(root, autoindex, index, accessMethods);

	//_________________________CGI___________
	root = "www/cgi";
	autoindex = false;
	index = "index.html";
	accessMethods.clear();
	accessMethods.push_back("GET");
	accessMethods.push_back("POST");

	Location		loc_cgi(root, autoindex, index, accessMethods);
	/*


	//______________________DELETE___________
	root = "www/delete";
	autoindex = true;
	index = "index.html";
	accessMethods.clear();
	accessMethods.push_back("GET");
	accessMethods.push_back("DELETE");

	Location		loc_delete(root, autoindex, index, accessMethods);

	//________________________POST___________
	root = "www/post";
	autoindex = true;
	index = "index.html";
	accessMethods.clear();
	accessMethods.push_back("GET");
	accessMethods.push_back("POST");

	Location		loc_post(root, autoindex, index, accessMethods);
	*/

	//______________________SERVER___________
	host = "127.0.0.1";
	port = "9000";
	server_name = "test.ru";
	body_size = 10000;
	error_page[400] = "www/400.html";
	error_page[404] = "www/404.html";
	location["/"] = loc;
	//location["/kapouet"] = loc;
	location["/cgi-bin"] = loc_cgi;
	//location["/delete"] = loc_delete;
	//location["/post"] = loc_post;

	Server			serv(host, port, server_name, body_size, error_page, location);

	webserver.makeServer(serv);
//#endif

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
