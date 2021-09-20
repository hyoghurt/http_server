# define DEBUG

#include "Webserver.hpp"
#include "add_funct.hpp"

int	main(int argc, char **argv)
{
	Webserver				webserver;

#ifdef DEBUG
	webserver.makeServer("127.0.0.1", 8000);
	webserver.makeServer("127.0.0.1", 9000);
#endif

	//читаем конфиг файл______________________________________________

	if (-1 == webserver.createSocketListen())
		return (1);

	if (-1 == webserver.start())
		return (1);

	return (0);
}
