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

		Client(const int& socket, std::vector<Server>& server) : socket(socket), server(server)
		{
			time(&timeStart);
			request = "";
			response = "";
		}

		/*
		Client(const Client& oth)	{ *this = oth; }
		*/

		~Client()					{ }

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

		void	readSocket(void)
		{
			std::string		str;

			//читаем данные с клиентского сокета в buf____________________________
			readByte = recv(socket, buf, BUF_SIZE - 1, 0);

			debagPrintReadByte();

			if (readByte > 0)
			{
				buf[readByte] = '\0';
				std::cout << buf << std::endl;
				//request += std::string(buf);
				//std::cout << request << std::endl;
				//cl_socket.request.append(buf);
				//проверяем получили ли все данные_______________________________
				//создаем ответ и отплавляем_____________________________________
				//create_response(cl_socket.request);
			}

			if (readByte < 0)
			{
				std::cout << "close fd read" << std::endl;
				shutdown(socket, 0);
			}
			//debag_pring_request(cl_socket.fd, cl_socket.request);
		}
			
			/*
			std::string	response_step_4(std::map<std::string, std::string> &jsn)
			{
				std::string		response;
				return (response);
			}
			
			//проверка существования файла______________________________________
			std::string	response_step_3(std::map<std::string, std::string> &jsn)
			{
				std::string		response;
				std::string		path;
			
				path = jsn["path"];
			
				if (path == "\\")
				{
				}
			
				return (response);
			}
			
			//проверка метода__________________________________________________
			std::string	response_step_2(std::map<std::string, std::string> &jsn)
			{
				std::string		response;
				std::string		method;
			
				method = jsn["method"];
				if (method == "GET" || method == "POST" || method == "DELETE")
					return (response_step_3(jsn));
			
				response = "501 Not Implemented";
				response += "\r\n";
				response += "Allow: GET, POST, DELETE";
				response += "\r\n\r\n";
			
				return (response);
			}
			
			//проверка синтаксиса_______________________________________________
			std::string	response_step_1(std::map<std::string, std::string> &jsn)
			{
				std::string		response;
				std::string		method;
				std::string		path;
				std::string		http_version;
			
				method = jsn["method"];
				path = jsn["path"];
				http_version = jsn["http_version"];
			
				if (method.length() != 0 && path.length() != 0 && http_version.length() != 0)
					return (response_step_2(jsn));
			
				response = "400 Bad Request";
				response += "\r\n\r\n";
			
				return (response);
			}
			
			std::string	create_response(const std::string& request)
			{
				std::string										response;
				std::map<std::string, std::string>				jsn;
				std::map<std::string, std::string>::iterator	it_jsn;
			
				jsn = return_map_request(request);
			
				for (it_jsn = jsn.begin(); it_jsn != jsn.end(); ++it_jsn)
					std::cout << (*it_jsn).first << " => " << (*it_jsn).second << std::endl;
			
				//exit(0);
			
				return (response);
				//return (response_step_1(jsn));
			}
			
			*/
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
				jsn["path"] = request.substr(start_n, pos_n - start_n);
			
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
		int						socket;
		time_t					timeStart;
		std::string				request;
		std::string				response;
		char					buf[BUF_SIZE];
		int						readByte;
		std::vector<Server>&	server;
		std::map<std::string, std::string>	jsn_request;
};

#endif
