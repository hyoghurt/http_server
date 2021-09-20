#ifndef SERVER_HPP
# define SERVER_HPP

# include "add_funct.hpp"
# include "def_color.hpp"

# include <iostream> //std
//# include <sys/socket.h>
//# include <sys/types.h>
//# include <netinet/in.h>
# include <arpa/inet.h> //inet_addr, sockaddr_in
# include <unistd.h> //close
# include <fcntl.h> //fntnl

class	Server
{
	public:
		Server() : socketListen(-1) {}
		Server(const Server &oth)
		{ *this = oth; }
		~Server()
		{
			if (socketListen != -1)
			{
				std::cout << "close socket listen: " << socketListen << std::endl;
				close (socketListen);
			}
		}

		Server&	operator= (const Server &oth)
		{
			this->socketListen = oth.socketListen;
			this->ipAddress = oth.ipAddress;
			this->port = oth.port;
			return *this;
		}

		int					getSocketListen(void) const
		{ return this->socketListen; }

		std::string			getIpAddress(void) const
		{ return this->ipAddress; }

		int					getPort(void) const
		{ return this->port; }

		void				setSocketListen(const int& socket)
		{ this->socketListen = socket; }

		void				setIpAddress(const std::string& str)
		{ this->ipAddress = str; }

		void				setPort(const int& port)
		{ this->port = port; }

		int					createSocketListen(void)
		{
			//создание сокета ___________________________________________________
			socketListen = socket(AF_INET, SOCK_STREAM, 0);
			if (socketListen == -1)
			{
				print_error("socket");
				return (-1);
			}

			//перевод сокета в неблокирующий режим____________________________
			int	flags = fcntl(socketListen, F_GETFL);
			if (-1 == fcntl(socketListen, F_SETFL, flags | O_NONBLOCK))
			{
				print_error("fcntl");
				return (-1);
			}

			//установили флаг, для предотвращения залипаниz TCP порта__________
			int	opt = 1;
			if (-1 == setsockopt(socketListen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
			{
				print_error("setsockopt");
				return (-1);
			}

			//сопостовление созданному сокету конкретного адреса________________
			struct sockaddr_in  addr;

			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = inet_addr(ipAddress.c_str());
			if (-1 == bind(socketListen, (struct sockaddr*) &addr, sizeof(addr)))
			{
				print_error("bind");
				return (-1);
			}

			//переводим сокет в слушаюший режим_________________________________
			if (-1 == listen(socketListen, 5))
			{
				print_error("listen");
				return (-1);
			}

			//печать на консоль информацию______________________________________
			std::cout << PINK << "________START SERVER_:" << socketListen << " ";
			std::cout << ipAddress << ":" << port << "    " << NO_C << '\n';

			return (socketListen);
		}

	private:
		//слушающий сокет_______________________________________
		int					socketListen;
		//ip адресс_____________________________________________
		std::string			ipAddress;
		//порт__________________________________________________
		int					port;
};

#endif
