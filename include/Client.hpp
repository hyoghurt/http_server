#ifndef CLIENT_HPP
# define CLIENT_HPP

# define BUF_SIZE 1024

# include <iostream>

class	Client
{
	public:
		/*
		Client()
		{
			socket = 0;
			time(&timeStart);
			request = "";
			response = "";
		}
		*/

		Client(const int& socket) : socket(socket)
		{
			time(&timeStart);
			request = "";
			response = "";
			responseHeader["Status"];
			responseHeader["Host"];
			responseHeader["Content-Length"];
			responseHeader["Connection"];
		}

		/*
		Client(const Client& oth)	{ *this = oth; }
		*/

		~Client()
		{
			std::cout << "distrutor client" << std::endl;
		}

		/*
		Client&	operator= (const Client& oth)
		{
			this->socket = oth.socket;
			this->timeStart = oth.timeStart;
			return *this;
		}
		*/

		int		getSocket(void) const
		{ return this->socket; }

		time_t	getTimeStart(void) const
		{ return this->timeStart; }

		std::string	getRequest(void) const
		{ return this->request; }

		std::string getResponse(void) const
		{ return this->response; }

		void	setSocket(const int &socket)
		{ this->socket = socket; }

		void	setTimeStart(void)
		{ ctime(&timeStart); }

		void	setRequest(const std::string &request)
		{ this->request = request; }

		void	setResponse(const std::string &response)
		{ this->response = response; }

		void	debagPrintReadByte(void)
		{
			std::cout << YELLOW << get_new_time() << " socketClient:" << socket;
			std::cout << " get_byte:" << readByte << RESET << '\n';
		}

		std::map<std::string, std::string>	return_map_request(const std::string &request)
		{
			size_t								pos;
			size_t								pos_n;
			size_t								start(0);
			size_t								start_n(0);
			std::string							tmp;
			std::map<std::string, std::string>	jsn;
			std::string							key;
			std::string							value;

			pos_n = request.find(" ", start_n);
			jsn["method"] = request.substr(start_n, pos_n - start_n);

			start_n = pos_n + 1;
			pos_n = request.find(" ", start_n);
			jsn["request_target"] = request.substr(start_n, pos_n - start_n);
		
			start_n = pos_n + 1;
			pos_n = request.find("\r", start_n);
			jsn["http_version"] = request.substr(start_n, pos_n - start_n);
		
			start_n = pos_n + 2;
			pos_n = request.find("\n", start_n);
		
			while (pos_n != std::string::npos)
			{
				tmp = request.substr(start_n, pos_n - start_n - 1);
		
				if (tmp.length() == 0)
					break;
		
				start = 0;
				pos = tmp.find(":", start);
				key = tmp.substr(start, pos - start);
		
				start = pos + 2;
				value = tmp.substr(start);
				jsn[key] = value;
		
				start_n = pos_n + 1;
				pos_n = request.find("\n", start_n);
			}
		
			return (jsn);
		}


	public:
		int									socket;
		time_t								timeStart;
		std::string							request;
		std::string							response;
		std::string							header;
		std::string							body;
		char								buf[BUF_SIZE];
		int									readByte;
		std::map<std::string, std::string>	json_request;
		size_t								byte_send;
		Server*								server;
		Location*							location;
		std::string							path_file;
		char*								buf_write;
		int									bytes_write;
		int									total_bytes_write;
		std::map<std::string, std::string>	responseHeader;
};

#endif
