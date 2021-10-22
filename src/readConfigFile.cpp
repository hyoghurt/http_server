# include "Webserver.hpp"

# define CONF_DEFAULT "tester/ft_tester/conf.conf"
//# define CONF_DEFAULT "tester/my_tester/conf.conf"

//CONF_PARSER__________________________________________________________________
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
						print_error("No founf locatio for server");
						return (-1);
					}
					server.location.push_back(location);
					setServer(server);
				}

				location.clear();
				f_location = false;
				server.clear();
				f_server = true;
			}
			else if (f_server && !f_location && res[0] == "host:" && res.size() == 2 && checkCorrectIP(res[1]))
				server.ipAddress = res[1];

			else if (f_server && !f_location && res[0] == "port:" && res.size() == 2 && checkCorrectHost(res[1]))
				server.port = res[1];

			else if (f_server && !f_location && res[0] == "server_name:" && res.size() == 2)
				server.serverName = res[1];

			else if (f_server && !f_location && res[0] == "client_max_body_size:" && res.size() == 2)
				server.clientMaxBodySize = atoi(res[1].c_str());

			else if (f_server && !f_location && res[0] == "error_page:" && res.size() == 3)
				server.errorPage[ atoi(res[1].c_str()) ] = res[2];

			else if (f_server && res[0] == "location" && res.size() == 2)
			{
				if (f_location)
					server.location.push_back(location);

				location.clear();
				f_location = true;
				location.rule = res[1];
			}

			else if (f_server && f_location && res[0] == "root:" && res.size() == 2)
				location.root = res[1];

			else if (f_server && f_location && res[0] == "index:" && res.size() == 2)
				location.index = res[1];

			else if (f_server && f_location && res[0] == "autoindex:" && res.size() == 2)
			{
				if (res[1] == "on")
					location.autoindex = true;
			}

			else if (f_server && f_location && res[0] == "access_method:" && checkCorrectMethodName(res)){
				for (int i = 1; i != res.size(); ++i){
					location.accessMethods.push_back(res[i]);
				}
			}

			else if (f_server && f_location && res[0] == "cgi_pass:" && res.size() == 2)
				location.cgiPass = res[1];

			else if (f_server && f_location && res[0] == "client_max_body_size:" && res.size() == 2)
				location.clientMaxBodySize = atoi(res[1].c_str());

			else if (f_server && f_location && res[0] == "return:" && res.size() == 3)
			{
				location.return_code = atoi(res[1].c_str());
				location.return_location = res[2];
			}
			else {
				print_error("Invalid field : " + line);
				myfile.close();
				return (-1);
			}
		}

		server.location.push_back(location);
		setServer(server);

		add_client_max_body_size();
		myfile.close();
		if (!checkСorrectField()){
			print_error("Invalid configuration file: " + file);
			return (-1);
		}
	}
	else
	{
		print_error("No founf config file: " + file);
		return (-1);
	}

	return (0);
}

bool Webserver::checkCorrectIP(std::string ip){
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

bool Webserver::checkCorrectHost(std::string host){
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
	for (int i = 1; i < names.size(); ++i){
		if (names[i] != "GET" && names[i] != "POST" && names[i] != "PUT" && names[i] != "HEAD" && names[i] != "DELETE")
			return false;
	}
	return true;
}

bool	Webserver::checkСorrectField(){
	std::vector<Server>::iterator			it_s;
	std::vector<Server>::iterator			it_ss;

	if (server.empty())
		return false;
	for (it_s = server.begin(); it_s != server.end(); ++it_s)
	{
		if ((*it_s).ipAddress.empty())
			return false;
		if ((*it_s).port.empty())
			return false;
		if ((*it_s).location.empty())
			return false;
		for (it_ss = it_s + 1; it_ss != server.end(); ++it_ss)
		{
			if ((*it_s).ipAddress == (*it_ss).ipAddress && (*it_s).port == (*it_ss).port && (*it_s).serverName == (*it_ss).serverName)
				return false;
		}
	}
	return true;
}
//ADD_CLIENT_MAX_BODY________________________________________________________
void	Webserver::add_client_max_body_size()
{
	std::vector<Server>::iterator		it;
	std::vector<Location>::iterator		lc;

	for (it = server.begin(); it != server.end(); ++it)
	{
		for (lc = (*it).location.begin(); lc != (*it).location.end(); ++lc)
		{
			if ((*lc).clientMaxBodySize == -1)
				(*lc).clientMaxBodySize = (*it).clientMaxBodySize;
		}
	}
}
//CONF_SHOW___________________________________________________________________
void	Webserver::debug_show_conf()
{
	std::cout << "____SHOW_CONF_______________________\n";
	std::vector<Server>::iterator			it_s;
	std::vector<Location>::iterator			it_l;
	std::map<int, std::string>::iterator	it_m;
	std::vector<std::string>::iterator		it_v;

	for (it_s = server.begin(); it_s != server.end(); ++it_s)
	{
		std::cout << "host = [" << (*it_s).ipAddress << "]\n";
		std::cout << "port = [" << (*it_s).port << "]\n";
		std::cout << "server_name = [" << (*it_s).serverName << "]\n";
		std::cout << "client_max_size = [" << (*it_s).clientMaxBodySize << "]\n";

		for (it_m = (*it_s).errorPage.begin(); it_m != (*it_s).errorPage.end(); ++it_m)
			std::cout << "error_page = [" << (*it_m).first << "]=[" << (*it_m).second << "]\n";

		for (it_l = (*it_s).location.begin(); it_l != (*it_s).location.end(); ++it_l)
		{
			std::cout << "----------------------------\n";
			std::cout << "rule = [" << (*it_l).rule << "]\n";
			std::cout << "root = [" << (*it_l).root << "]\n";
			std::cout << "autoindex = [" << (*it_l).autoindex << "]\n";
			std::cout << "index = [" << (*it_l).index << "]\n";

			for (it_v = (*it_l).accessMethods.begin(); it_v != (*it_l).accessMethods.end(); ++it_v)
				std::cout << "method = [" << *it_v << "]\n";

			std::cout << "return_code = [" << (*it_l).return_code << "]\n";
			std::cout << "return_location = [" << (*it_l).return_location << "]\n";
			std::cout << "cgiPass = [" << (*it_l).cgiPass << "]\n";
			std::cout << "client_max_body_size = [" << (*it_l).clientMaxBodySize << "]\n";
		}
	}
	std::cout << "____________________________________\n";
}
