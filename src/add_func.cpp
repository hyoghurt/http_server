#include "add_func.hpp"

void	print_debug(const std::string& str)
{ std::cout << BLUE << str << RESET << '\n'; }

void	print_info(const std::string& str)
{ std::cout << YELLOW << str << RESET << '\n'; }

void	print_error(const std::string& str)
{ std::cerr << RED << "webserver: Error: " << str << RESET << '\n'; }

void	print_error_strerror(const std::string& str)
{ std::cerr << RED << "webserver: Error: " << str << ": " << strerror(errno) << RESET << '\n'; }

void	print_connect_info(int socket_listen, int fd_cl, struct sockaddr_in addr_cl)
{
	std::cout << GREEN;
	std::cout << "Connect: in socketListen " << socket_listen << " out ";
	std::cout << inet_ntoa(addr_cl.sin_addr) << ":";
	std::cout << ntohs(addr_cl.sin_port) << "  ";
	std::cout << get_new_time();
	std::cout << " socketClient: " << fd_cl << NO_C << '\n';
}

std::string		get_new_time()
{
	struct timeval	tv;
	struct tm		*info;
	char			buffer[64];
 
	gettimeofday(&tv, nullptr);
	info = localtime(&tv.tv_sec);
	strftime (buffer, sizeof buffer, "%x %X %Y", info);

	return (buffer);
}

int		check_dir_or_file(const std::string& name_file)
{
	struct stat		sb;

	if (-1 == stat((name_file).c_str(), &sb))
		return (-1);
	if (S_ISDIR(sb.st_mode))
		return (1);
	return (0);
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
