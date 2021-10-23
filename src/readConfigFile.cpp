# include "Webserver.hpp"

int		Webserver::readConfigFile(const char* fileName)
{
	std::string					file;

	if (!fileName)
		file = CONF_DEFAULT;
	else
		file = fileName;

	std::ifstream				myfile (file, std::ios::out);
	std::string					line;
	std::vector<std::string>	res;
	Server						server;
	Location					location;
	bool						f_server(false);
	bool						f_location(false);

	if (myfile.is_open())
	{
		while (getline(myfile,line))
		{
			res = split_by_space(line);

			if (res.empty())
				continue ;
			if (res[0] == "server" && res.size() == 1)
			{
				if (f_server)
				{
					if (!f_location)
					{
						myfile.close();
						return (print_error("No founf locatio for server"));
					}
					server.setLocation(location);
					this->server.push_back(server);
				}
				location.clear();
				f_location = false;
				server.clear();
				f_server = true;
			}

			else if (f_server && !f_location && res[0] == "host:"
					&& res.size() == 2 && checkCorrectIP(res[1]))
				server.setIpAddress(res[1]);

			else if (f_server && !f_location && res[0] == "port:"
					&& res.size() == 2 && checkCorrectHost(res[1]))
				server.setPort(res[1]);

			else if (f_server && !f_location
					&& res[0] == "server_name:" && res.size() == 2)
				server.setServerName(res[1]);

			else if (f_server && !f_location
					&& res[0] == "client_max_body_size:" && res.size() == 2)
				server.setClientMaxBodySize(atoi(res[1].c_str()));

			else if (f_server && !f_location
					&& res[0] == "error_page:" && res.size() == 3)
				server.setErrorPage(atoi(res[1].c_str()), res[2]);

			else if (f_server && res[0] == "location" && res.size() == 2)
			{
				if (f_location)
					server.setLocation(location);
				location.clear();
				f_location = true;
				location.setRule(res[1]);
			}

			else if (f_server && f_location
					&& res[0] == "root:" && res.size() == 2)
				location.setRoot(res[1]);

			else if (f_server && f_location
					&& res[0] == "index:" && res.size() == 2)
				location.setIndex(res[1]);

			else if (f_server && f_location && res[1] == "on"
					&& res[0] == "autoindex:" && res.size() == 2)
				location.setAutoindex(true);

			else if (f_server && f_location && res[0] == "access_method:"
					&& checkCorrectMethodName(res))
				for (int i = 1; i != res.size(); ++i)
					location.setAccessMethods(res[i]);

			else if (f_server && f_location
					&& res[0] == "cgi_pass:" && res.size() == 2)
			{
				location.setCgiPass(absolutePathOfExec(res[1]));
				if (location.getCgiPass() == "")
				{
					myfile.close();
					return (print_error("Invalid cgi_pass path"));
				}
			}

			else if (f_server && f_location
					&& res[0] == "client_max_body_size:" && res.size() == 2)
				location.setClientMaxBodySize(atoi(res[1].c_str()));

			else if (f_server && f_location
					&& res[0] == "return:" && res.size() == 3)
				location.setReturn(atoi(res[1].c_str()), res[2]);

			else
			{
				myfile.close();
				return (print_error("Invalid field: " + line));
			}
		}

		server.setLocation(location);
		this->server.push_back(server);
		myfile.close();

		if (!checkСorrectField())
			return (print_error("Invalid configuration file: " + file));

		addClientMaxBodySize();
	}
	else
		return (print_error("No founf config file: " + file));

	return (0);
}

bool	Webserver::checkCorrectIP(std::string ip){
	std::vector<int> tmp;
	std::string number = "";

	for (int i = 0; i < ip.size(); i++)
	{
		if (ip[i] == '.' && i != 0 && ip[i - 1] != '.'){
			tmp.push_back(stoi(number));
			number = "";
		} else if (isdigit(ip[i])) {
			number += ip[i];
			if (i == ip.size() - 1){
				tmp.push_back(stoi(number));
			}
		} else {
			return false;
		}
	}
	if (tmp.size() != 4){
		return false;
	}
	for (int i = 0; i < tmp.size(); ++i){
		if (tmp[i] < 0 || tmp[i] > 255)
			return false;
	}
	return true;
}

bool	Webserver::checkCorrectHost(std::string host){
	for (int i = 0; i < host.size(); i++)
	{
		if (!isdigit(host[i]))
			return false;
	}
	int tmp = stoi(host);
	if (tmp < 0 || tmp > 65535)
		return false;
	return true;
}

bool	Webserver::checkCorrectMethodName(std::vector <std::string> names){
	for (int i = 1; i < names.size(); ++i)
		if (names[i] != "GET" && names[i] != "POST" && names[i] != "DELETE"
				&& names[i] != "HEAD" && names[i] != "PUT")
			return false;
	return true;
}

bool	Webserver::checkСorrectField(){
	std::vector<Server>::iterator			it_s;
	std::vector<Server>::iterator			it_ss;

	if (server.empty())
		return false;
	for (it_s = server.begin(); it_s != server.end(); ++it_s)
	{
		if ((*it_s).getIpAddress().empty())
			return false;
		if ((*it_s).getPort().empty())
			return false;
		if ((*it_s).checkLocationEmpty())
			return false;
		for (it_ss = it_s + 1; it_ss != server.end(); ++it_ss)
			if ((*it_s).getIpAddress() == (*it_ss).getIpAddress()
					&& (*it_s).getPort() == (*it_ss).getPort()
					&& (*it_s).getServerName() == (*it_ss).getServerName())
				return false;
		(*it_s).multiServerName();
	}
	return true;
}

void	Webserver::addClientMaxBodySize()
{
	std::vector<Server>::iterator		it;

	for (it = server.begin(); it != server.end(); ++it)
		(*it).addClientMaxBodySize((*it).getClientMaxBodySize());
}
