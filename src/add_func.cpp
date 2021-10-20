#include "add_func.hpp"

void	print_debug(const std::string& str)
{ return; std::cout << BLUE << str << RESET << '\n'; }

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

std::string		convert_base16_to_str(const size_t& n)
{
	std::stringstream	ss;
	std::string			str;

	ss << std::hex << n;
	ss >> str;

	return (str);
}

size_t			convert_str_to_base16(const std::string& str)
{
	size_t		n;

	std::istringstream(str) >> std::hex >> n;
	return (n);
}

std::vector<std::string>	split_by_space(const std::string& str)
{
   std::vector<std::string>		res;
   std::string const delims =  " \t";

   size_t begin, pos = 0;
   while ((begin = str.find_first_not_of(delims, pos)) != std::string::npos)
   {
      pos = str.find_first_of(delims, begin + 1);
      res.push_back(str.substr(begin, pos - begin));
   }
   return res;
}

std::string		absolutePathOfExec(const std::string& execName)
{
	std::string		absolutePath;
	std::string		token;
	std::string		path = getenv("PATH");
	size_t			pos = 0;
	struct stat		st;

	if (path.empty())
		return ("");

	while ((pos = path.find(':')) != std::string::npos)
	{
		token = path.substr(0, pos);
		absolutePath = (token.append("/") + execName);

		if (stat(absolutePath.c_str(), &st) == 0)
			return absolutePath;

		path.erase(0, pos + 1);
	}
	return ("");
}

int				check_host(const std::string &host)
{
   int positions_after_point[5] = {0};

   try {
      //Ищем все индексы точек и двоеточия и записываем их в массив интов
      for (int i = 0, j = 1; i != host.length(); i++) {
         if (j < 4 && host[i] == '.') {
            positions_after_point[j++] = i + 1;
         }
         else if (host[i] == ':') {
            if (j != 4) { return 2; } //more than 4 elem
            positions_after_point[j++] = i + 1;
         }
         else if (!isdigit(host[i])) { return 1; }
      }

      //Для каждого индекса делает сабстроку и переводим ее в инт
      for (int i = 0; i < 5; i++) {
         std::string b = host.substr(positions_after_point[i],
                              positions_after_point[i + 1] - positions_after_point[i] - 1);
         int res = stoi(b);
         if (i < 4 && (res < 0 || res > 255)) { return 3; } //elem go from range
//       std::cout << res << std::endl;
      }
   }
   catch (...) { return 4; }
   return 0;
}
