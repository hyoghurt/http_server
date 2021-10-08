#ifndef CLIENT_HPP
# define CLIENT_HPP

# define BUF_SIZE 24
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

		Client(const int& socket, char* addr) : socket(socket), addr(addr)
		{
			time(&timeStart);
			request = "";
			response = "";
			readByte = 0;
			cgi_f = 0;
		}

		Client(const Client& oth)	{ *this = oth; }

		~Client()
		{
			std::cout << "distrutor client" << std::endl;
		}

		Client&	operator= (const Client& oth)
		{
			this->socket = oth.socket;
			this->timeStart = oth.timeStart;
			this->request = oth.request;
			this->response = oth.response;
			this->header = oth.header;
			this->body = oth.body;
			this->readByte = oth.readByte;
			this->json_request = oth.json_request;
			this->server = oth.server;
			this->location = oth.location;
			this->path_file = oth.path_file;
			this->responseHeader = oth.responseHeader;
			this->envCgi = oth.envCgi;
			this->interpreter = oth.interpreter;
			this->addr = oth.addr;
			this->cgi_f = oth.cgi_f;
			return *this;
		}

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


		std::pair<std::string, std::string>		return_key_val(const std::string& str)
		{
			int				start(0);
			int				end;
			std::string		key;
			std::string		val;

			end = str.find(":");
			if (end != std::string::npos)
			{
				key = str.substr(start, end - start);
				val = str.substr(end + 2);
			}

			return (std::pair<std::string, std::string>(key, val));
		}

		void	create_json_request(void)
		{
			size_t			end;
			size_t			start(0);
			std::string		tmp;

			end = request.find("\r\n\r\n");
			if (end == std::string::npos)
				return ;
			json_request["body"] = request.substr(end + 4);
			request.erase(end);

			end = request.find(" ", start);
			if (end == std::string::npos)
				return ;
			json_request["method"] = request.substr(start, end - start);

			start = end + 1;
			end = request.find(" ", start);
			if (end == std::string::npos)
				return ;
			json_request["request_target"] = request.substr(start, end - start);

			start = end + 1;
			end = request.find("\r", start);
			if (end == std::string::npos)
				return ;
			json_request["http_version"] = request.substr(start, end - start);

			start = end + 2;
			end = request.find("\n", start);
			if (end == std::string::npos)
				return ;
			while (end != std::string::npos)
			{
				tmp = request.substr(start, end - start - 1);
		
				json_request.insert(return_key_val(tmp));
		
				start = end + 1;
				end = request.find("\n", start);
			}

			tmp = request.substr(start);
			json_request.insert(return_key_val(tmp));
		}


		void	deb()
		{
			std::map<std::string, std::string>::iterator	it;

			for (it = json_request.begin(); it != json_request.end(); ++it)
			{
				std::cout << (*it).first << "=" << (*it).second << std::endl;
			}
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

		int		check_413(void)
		{
			int				size;

			if (0 == server->clientMaxBodySize)
				return (0);

			if (!json_request["Content-Length"].empty())
			{
				size = atoi(json_request["Content-Length"].c_str());
				if (size > server->clientMaxBodySize)
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
			//if (!cgi_f)
			header += "\r\n";

			response = header + body;
		}

		int		open_file(const std::string& str)
		{
			print_debug("F open file: " + str);

			int				length;
			char*			buffer;
			int				status_code;

			status_code = 200;

			std::ifstream	is (str, std::ifstream::binary);

			if (is)
			{
				debug_info("open file: " + str);

				is.seekg (0, is.end);
				length = is.tellg();
				is.seekg (0, is.beg);

				try
				{
					buffer = new char [length];
				}
				catch (std::bad_alloc& ba)
				{
					print_error("open_file: bad_alloc");
					return (500);
				}

				debug_info("size file: " + std::to_string(length));

				is.read (buffer,length);

				if (is)
				{
					body.assign(buffer, length);
					debug_info("all characters read successfully");
				}
				else
				{
					status_code = 500;
					debug_info("Error: read file (only " + std::to_string(is.gcount()) + " could be read)");
				}

				is.close();
				delete [] buffer;
			}
			else
				status_code = 404;

			return (status_code);
		}

		int				post_run(void)
		{
			print_debug("F post file: " + path_file);

			std::ofstream	myfile(path_file, std::ios::binary);

			if (!myfile)
			{
				debug_info(path_file + " no open");
				return (500);
			}
			myfile << json_request["body"];
			myfile.close();
			debug_info(path_file + " mod");
			return (201);
		}

		int				get_run(void)
		{
			print_debug("F method GET");

			DIR									*dp;
			struct dirent						*ep;
			std::vector<std::string>			name_file;
			int									status_code;
			int									found;

			status_code = open_file(path_file);

			if (location->autoindex && status_code == 404)
			{
				print_debug("F autoindex true");

				found = path_file.find(location->index);
				if (found == std::string::npos)
					return (404);
				path_file.erase(found);

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
			}
			return (status_code);
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

		int		autoindex_rename_file(std::vector<std::string>& name_file)
		{
			std::vector<std::string>::iterator	it;
			int									ret;

			it = name_file.begin() + 2;

			for (; it != name_file.end(); ++it)
			{
				ret = check_dir_or_file(path_file + *it);
				if (ret == 1)
					(*it).push_back('/');
				else if (ret == -1)
					return (-1);
			}
			return (0);
		}

		void	autoindex_create_body(std::vector<std::string>& name_file)
		{
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

		void	cgi_env(void)
		{
			print_debug("F cgi env");

			//envCgi.clear();
			//envCgi["AUTH_TYPE"] = "";		//delete
			envCgi["CONTENT_LENGTH"] = json_request["Content-Length"];
			envCgi["CONTENT_TYPE"] = json_request["Content-Type"];
			envCgi["GATEWAY_INTERFACE"] = "CGI/0.1";
			envCgi["PATH_TRANSLATED"] = "./" + location->root + envCgi["PATH_INFO"];
			envCgi["REMOTE_ADDR"] = addr;
			//envCgi["REMOTE_HOST"] = "";		//delete
			//envCgi["REMOTE_IDENT"] = "";	//delete
			//envCgi["REMOTE_USER"] = "";		//delete
			envCgi["REQUEST_METHOD"] = json_request["method"];
			envCgi["SCRIPT_NAME"] = json_request["request_target"];		//!!! write
			envCgi["SERVER_NAME"] = server->serverName;
			envCgi["SERVER_PORT"] = server->port;
			envCgi["SERVER_PROTOCOL"] = "HTTP/1.1";
			envCgi["SERVER_SOFTWARE"] = "La Femme Nikita 0.1";
		}

		char**	cgi_create_env(void)
		{
			print_debug("F create env");

			std::map<std::string, std::string>::iterator	it;
			int												i;

			cgi_env();

			try
			{
				char											**env;
				env = new char*[envCgi.size() + 1];

				i = 0;
				for (it = envCgi.begin(); it != envCgi.end(); ++it)
				{
					env[i] = strdup(( (*it).first + "=\"" + (*it).second + "\"").c_str());
					if (!env[i])
						return (free_env(env));
					++i;
					std::cout << env[i] << std::endl;
				}
				env[i] = 0;

				std::cout << "sdffffffffffffffffffffffffffffffffffflsdfldsfdsf" << std::endl;
				dech(env);
				std::cout << "sdffffffffffffffffffffffffffffffffffflsdfldsfdsf" << std::endl;

				return (env);
			}
			catch (std::bad_alloc& ba)
			{
				print_error("cgi_create_env: bad_alloc");
				return (0);
			}


		}

		char**	cgi_create_arg(void)
		{
			char	**arg;

			try
			{
				arg = new char*[3];

				arg[0] = strdup(interpreter.c_str());
				if (!arg[0])
					return (free_env(arg));
				arg[1] = strdup(path_file.c_str());
				if (!arg[1])
					return (free_env(arg));
				arg[2] = 0;
			}
			catch (std::bad_alloc& ba)
			{
				print_error("cgi_create_env: bad_alloc");
				return (0);
			}

			return (arg);
		}

		char**	free_env(char** env)
		{
			print_debug("F free env");
			int		i = 0;

			if (!env)
				return (0);
			while (env[i])
				free(env[i++]);
			delete [] env;
			return (0);
		}

		void	de()
		{
			std::map<std::string, std::string>::iterator	it;

			for (it = envCgi.begin(); it != envCgi.end(); ++it)
			{
				std::cout << (*it).first << "=" << (*it).second << std::endl;
			}
		}

		void	dech(char** env)
		{
			std::cout << "dddddddddddddddddddddddddddddeccccccch" << std::endl;
			int		i = 0;

			std::cout << env << std::endl;
			std::cout << *env << std::endl;

			while (env[i] != NULL)
			{
				std::cout << "1" << std::endl;
				std::cout << env[i] << std::endl;
				++i;
			}
		}

		int		cgi_run(void)
		{
			print_debug("F run cgi");

			cgi_f = 1;

			char	**arg = cgi_create_arg();
			char	**env = cgi_create_env();
			int		status_code;

			dech(env);

			status_code = 500;

			if (arg && env)
				status_code = cgi_fork(arg, env);

			free_env(arg);
			free_env(env);

			return (status_code);
		}

		int		cgi_fork(char** arg, char** env)
		{
			int		status;
			pid_t	pid;
			int		pipefd[2];
			int		status_code;

			status_code = 200;

			pipe(pipefd);

			pid = fork();

			if (pid == -1)
			{
				print_error("fork");
				status_code = 500;
				return (500);
			}
			if (pid == 0)
			{
				close(pipefd[0]);
				dup2(pipefd[1], 1);
				close(pipefd[1]);

				execve(arg[0], arg, env);
				exit (errno);
			}

			wait(&status);

			close(pipefd[1]);

			if (WEXITSTATUS(status) != 0)
			{
				status_code = 500;
				print_error("execve exit 0");
			}
			else if (WEXITSTATUS(status) == 2)
			{
				status_code = 404;
				print_info("execve exit 2");
			}
			else
				cgi_read(pipefd[0]);

			close(pipefd[0]);

			return (status_code);
		}


		void	cgi_read(int fd)
		{
			int		bytes;

			body.clear();

			bytes = read(fd, buf, BUF_SIZE - 1);
			while (bytes > 0)
			{
				buf[bytes] = '\0';
				body.append(buf);
				bytes = read(fd, buf, BUF_SIZE - 1);
			}
		}

		int		deleteFile(void)
		{
			print_debug("F delete file: " + path_file);

			if (unlink(path_file.c_str()) == -1)
				return 404;

			debug_info(path_file + " was deleted");
			return 204;
		}

		void	debug_info(const std::string& mes)
		{ std::cout << YELLOW << get_new_time() << " " << "Cl:" << socket << " " << mes <<  RESET << '\n'; }



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
		Server*								server;
		Location*							location;
		std::string							path_file;
		std::map<std::string, std::string>	responseHeader;
		std::map<std::string, std::string>	envCgi;
		std::string							interpreter;
		std::string							addr;
		int									cgi_f;
};

#endif
