#include "Webserver.hpp"

int	main(int argc, char **argv)
{
	if (argc > 2)
		return (1);

	Webserver			webserver;

	if (-1 == webserver.readConfigFile(argv[1]))
		return (1);

	if (-1 == webserver.createSocketListen())
		return (1);

	if (-1 == webserver.start())
		return (1);

	return (0);
}
