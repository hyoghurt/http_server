#include "add_funct.hpp"

void			print_error(const std::string& str)
{
    std::cerr << "webserver: " << str << ": " << strerror(errno) << std::endl;
}

void			print_connect_info(int socket_listen, int fd_cl, struct sockaddr_in addr_cl)
{
	std::cout << GREEN;
	std::cout << "Connect: in socketListen " << socket_listen << " out ";
	std::cout << inet_ntoa(addr_cl.sin_addr) << ":";
	std::cout << ntohs(addr_cl.sin_port) << "  ";
	std::cout << get_new_time();
	std::cout << " socketClient: " << fd_cl << NO_C << '\n';
}

std::string		get_new_time(void)
{
	struct timeval	tv;
	struct tm		*info;
	char			buffer[64];
 
	gettimeofday(&tv, NULL);
	info = localtime(&tv.tv_sec);
	strftime (buffer, sizeof buffer, "%x %X %Y", info);

	return (buffer);
}
/*
void			debag_pring_request(const int& fd_client, const std::string& str)
{
	std::cout << GREEN << "________________" << fd_client << NO_C << std::endl;
	std::cout << str << std::endl;
	std::cout << GREEN << "________________" << NO_C << std::endl;
}

std::string		ret_str_open(const std::string& file)
{
	std::stringstream my_str;
	my_str << "lkjijk";
	std::cout << my_str.str() << std::endl;
	std::string			str;
	std::string			res_str;
	std::ifstream		ifs(file);

	if (!ifs)
		//write correct
		return ("error\n");
	while (ifs)
	{
		getline(ifs, str);
		res_str += str;
		if (ifs)
			res_str += "\n";
	}
	ifs.close();
	return (res_str);
}


*/
