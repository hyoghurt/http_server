#include "addFunctions.hpp"

int				print_error(const std::string& str)
{
	std::cerr << RED << "webserver: Error: " << str << RESET << '\n';
	return (-1);
}

int				print_error_strerror(const std::string& str)
{
	std::cerr << RED << "webserver: Error: " << str << ": "
	<< strerror(errno) << RESET << '\n';
	return (-1);
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

int				check_dir_or_file(const std::string& name_file)
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
   const std::string			delims(" \t");

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

	if (stat(execName.c_str(), &st) == 0)
		return (execName);

	if (path.empty())
		return ("");

	while ((pos = path.find(':')) != std::string::npos)
	{
		token = path.substr(0, pos);
		absolutePath = (token.append("/") + execName);

		if (stat(absolutePath.c_str(), &st) == 0)
			return (absolutePath);

		path.erase(0, pos + 1);
	}
	return ("");
}

std::string	get_status_code(const int& code)
{
	switch (code)
	{
		case 200:
			return ("200 OK");
		case 201:
			return ("201 Created");
		case 204:
			return ("204 No Content");
		case 301:
			return ("301 Moved Permanently");
		case 400:
			return ("400 Bad Request");
		case 404:
			return ("404 Not Found");
		case 405:
			return ("405 Method Not Allowed");
		case 413:
			return ("413 Payload Too Large");
		case 500:
			return ("500 Internal Server Error");
		case 501:
			return ("501 Not Implemented");
		default:
			return ("Not found");
	}
}

std::pair<std::string, std::string>	returnKeyVal(const std::string& str)
{
	size_t			start(0);
	size_t			end(0);
	std::string		key("");
	std::string		val("");

	start = str.find_first_not_of(" \t", start);
	end = str.find(":");
	if (end != std::string::npos)
	{
		key = str.substr(start, end - start);
		start = str.find_first_not_of(" \t", end + 1);
		val = str.substr(start);
	}
	return (std::pair<std::string, std::string>(key, val));
}
