#ifndef CLIENT_HPP
# define CLIENT_HPP

# define BUF_SIZE 1024
# define TIME_KEEP_ALIVE 5

# include <iostream>
# include <dirent.h> //opendir
# include <sys/stat.h> //stat or <sys/types.h> <unistd.h>

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

		void	debugPrintReadByte(void)
		{
			std::cout << YELLOW << get_new_time() << " socketClient:" << socket;
			std::cout << " get_byte:" << readByte << RESET << '\n';
		}

		void	create_json_request(void)
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

			json_request = jsn;
		}

		int		check_501(void)
		{
			std::string	method;

			method = json_request["method"];

			if (method != "GET" && method != "POST" && method != "DELETE")
			{
				responseHeader["Allow"] = "Allow: GET, POST, DELETE\r\n";
				return (1);
			}
			return (0);
		}

		int		check_405(void)
		{
			std::vector<std::string>::iterator	it;
			std::string							method;

			it = location->accessMethods.begin();

			method = json_request["method"];

			while (it != location->accessMethods.end())
			{
				if (*it == method)
					return (0);
				++it;
			}
			return (1);
		}

		void	response_header_connection(void)
		{
			if (json_request["Connection"] == "close")
				responseHeader["Connection"] = "Connection: close\r\n";
			else
			{
				responseHeader["Connection"] = "Connection: Keep-Alive\r\n";
				responseHeader["Keep-Alive"] = "Keep-Alive: timeout=" +
					std::to_string(TIME_KEEP_ALIVE) + "\r\n";
			}
		}

		void	response_total(void)
		{
			std::map<std::string, std::string>::iterator	it;

			response_header_connection();
			responseHeader["Host"] = "Host: " + json_request["Host"] + "\r\n";

			if (!body.empty())
			{
				responseHeader["Content-Length"] =
					"Content-Length: " +
					std::to_string(body.size()) + 
					"\r\n";
			}

			header = responseHeader["Status"];
			header += responseHeader["Host"];

			for (it = responseHeader.begin(); it != responseHeader.end(); ++it)
			{
				if ((*it).first == "Status")
					continue;
				if ((*it).first == "Host")
					continue;
				if (!(*it).second.empty())
					header += (*it).second;
			}
			header += "\r\n";

			response = header + body;
		}

		int		open_file(const std::string& str)
		{
			print_debug("open file: " + str);

			int				length;
			char*			buffer;
			int				status_code;

			status_code = 200;

			std::ifstream	is (str, std::ifstream::binary);

			if (is)
			{
				is.seekg (0, is.end);
				length = is.tellg();
				is.seekg (0, is.beg);

				buffer = new char [length];

				print_info(" Cl:" + std::to_string(socket) + " Reading characters: " + std::to_string(length));

				is.read (buffer,length);

				if (is)
				{
					body.assign(buffer, length);
					print_info(" Cl: " + std::to_string(socket) + " all characters read successfully");
				}
				else
				{
					status_code = 500;
					print_error("Error: only " + std::to_string(is.gcount()) + " could be read");
				}

				is.close();
				delete [] buffer;
			}
			else
				status_code = 404;

			return (status_code);
		}


		int				autoindex(void)
		{
			print_debug("F autoindex");

			DIR									*dp;
			struct dirent						*ep;
			std::vector<std::string>			name_file;
			std::vector<std::string>::iterator	it;
			int									status_code;

			status_code = open_file(path_file);

			if (location->autoindex && status_code == 404)
			{
				print_debug("autoindex true");

				path_file.erase(path_file.size() - location->index.size());

				dp = opendir(path_file.c_str());
				if (NULL != dp)
				{
					while (NULL != (ep = readdir(dp)))
						name_file.push_back(ep->d_name);
					closedir(dp);

					if (-1 == autoindex_rename_file(name_file))
						return (500);

					autoindex_create_body(name_file);
					return (200);
				}
				status_code = 404;
			}
			return (status_code);
		}

		int		autoindex_rename_file(std::vector<std::string>& name_file)
		{
			std::vector<std::string>::iterator	it;
			int									ret;
			struct stat							sb;

			it = name_file.begin() + 2;

			for (; it != name_file.end(); ++it)
			{
				if (-1 == stat((path_file + *it).c_str(), &sb))
					return (-1);
				if (S_ISDIR(sb.st_mode))
					(*it).push_back('/');
			}
			return (0);
		}

		void	autoindex_create_body(std::vector<std::string>& name_file)
		{
			print_debug("F autoindex create body");

			std::vector<std::string>::iterator	it;
			std::string&						dir = json_request["request_target"];

			body.assign("<html><head>\r\n");
			body.append("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\r\n");
			body.append("<title>Directory listing for " + dir + "</title>\r\n");
			body.append("</head>\r\n");
			body.append("<body>\r\n");
			body.append("<h1>Directory listing for " + dir + "</h1>\r\n");
			body.append("<hr>\r\n");
			body.append("<ul>\r\n");

			it = name_file.begin() + 2;
			for (; it != name_file.end(); ++it)
				body.append("<li><a href=\"" + *it + "\">" + *it + "</a></li>\r\n");
		
			body.append("</ul>\r\n");
			body.append("<hr>\r\n");
			body.append("</body></html>\r\n");
		}

		char**	create_env(void)
		{
			std::map<std::string, std::string>::iterator	it;

			int		size = envCgi.size();
			int		i = 0;

			if (size == 0)
				return (0);

			char	**env = new char*[size + 1];

			for (it = envCgi.begin(); it != envCgi.end(); ++it)
				env[i++] = strdup(( (*it).first + "=\"" + (*it).second + "\"").c_str());
			env[i] = NULL;

			return (env);
		}

		int	run_cgi(void)
		{
			print_debug("F run cgi");

			char	**arg = new char*[3];

			if (path_file.find(".py"))
			{
				arg[0] = strdup("/Users/hyoghurt/.brew/bin/python3");
				arg[1] = strdup(path_file.c_str());
			}
			arg[2] = NULL;

			/*
			for (int i = 0; arg[i] != NULL; ++i)
				std::cout << arg[i] << std::endl;
			*/

			char	**env = create_env();
			int		rv;
			pid_t	pid;
			int		pipefd[2];

			for (int i = 0; i != 2; ++i)
				std::cout << env[i] << std::endl;

			pipe(pipefd);

			pid = fork();

			if (pid == 0)
			{
				close(pipefd[0]);
				dup2(pipefd[1], 1);
				close(pipefd[1]);

				execve(arg[0], arg, env);
				return (errno);
			}

			for (int i = 0; arg[i] != NULL; ++i)
				delete arg[i];
			delete [] arg;

			for (int i = 0; env[i] != NULL; ++i)
				delete env[i];
			delete [] env;

			if (pid == -1)
			{
				print_error("fork");
				return (500);
			}

			wait(&rv);

			close(pipefd[1]);

			if (WEXITSTATUS(rv) != 0)
			{
				print_error("execve exit 0");
				close(pipefd[0]);
				return (500);
			}

			if (WEXITSTATUS(rv) == 2)
			{
				print_error("execve exit 2");
				close(pipefd[0]);
				return (404);
			}

			readByte = read(pipefd[0], buf, BUF_SIZE - 1);

			buf[readByte] = '\0';

			body.assign(buf, readByte);

			close(pipefd[0]);

			return (200);
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
		std::map<std::string, std::string>	envCgi;
		int									statusCode;
};

#endif
