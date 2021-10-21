#ifndef CLIENT_HPP
# define CLIENT_HPP

# define BUF_SIZE 10000
# define TIME_KEEP_ALIVE 3

# include <iostream>
# include <dirent.h> //opendir

class	Server;
class	Location;

class	Client
{
	public:
		Client(const int& socket, char* addr) : socket(socket), addr(addr)
		{
			time(&timeStart);
			request = "";
			response = "";
			readByte = 0;
			flag = 0;
			chunked = 0;
		}

		Client(const Client& oth)	{ *this = oth; }

		~Client()
		{ std::cout << "distrutor client" << std::endl; }

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
			this->addr = oth.addr;
			this->flag = oth.flag;
			this->chunked = oth.chunked;
			return *this;
		}

		int		getSocket() const
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
		{ time(&timeStart); }

		void	setRequest(const std::string &request)
		{ this->request = request; }

		void	setResponse(const std::string &response)
		{ this->response = response; }

		const std::string	getRequestTarget()
		{ 
			std::map<std::string, std::string>::iterator	it;

			it = json_request.find("request_target");
			if (it != json_request.end())
				return (*it).second;
			return ("");
		}

		const std::string	getRequestMethod()
		{ 
			std::map<std::string, std::string>::iterator	it;

			it = json_request.find("method");
			if (it != json_request.end())
				return (*it).second;
			return ("");
		}

		const std::string	getRequestHost(void)
		{ 
			std::map<std::string, std::string>::iterator	it;

			it = json_request.find("Host");
			if (it != json_request.end())
				return (*it).second;
			return ("");
		}

		void	setResponseHeaderStatus(const int& code)
		{ responseHeader["Status"] = get_status_code(code); }

		bool	checkResponseHeaderConnectionClose()
		{
			std::map<std::string, std::string>::iterator	it;

			it = responseHeader.find("Connection");
			if (it != responseHeader.end() && (*it).second == "close")
				return (true);
			return (false);
		}

		std::string	getPathErrorPage(const int& code) const
		{
			std::map<int, std::string>::iterator	it;

			if (server == nullptr)
				return ("");

			it = server->errorPage.find(code);
			if (it != server->errorPage.end())
				return (*it).second;
			return ("");
		}

		void			setServer(Server* serv)
		{ this->server = serv; }

		Server*			getServer() const
		{ return this->server; }

		void			setLocation(Location* loc)
		{ this->location = loc; }

		void			setResponseHeaderLocation(const std::string& str)
		{ responseHeader["Location"] = str; }

		const Location*	getLocation() const
		{ return this->location; }

		void				setPathFile(const std::string& str)
		{ this->path_file = str; }

		const std::string&	getPathFile() const
		{ return this->path_file; }

		Location*			getLocationFindRule(const std::string& rule)
		{ return (server->findLocationRule(rule)); }

//READ_SOCKET__________________________________________________________________
		void	erase_header_request()
		{
			std::map<std::string, std::string>::iterator	it;

			size_t	found = request.find("\r\n\r\n");

			if (found != std::string::npos)
			{
				create_json_request_header(request.substr(0, found));
				request.erase(0, found + 4);

				it = json_request.find("Content-Length");
				if (it != json_request.end())
					readByte = atoi((*it).second.c_str());

				it = json_request.find("Transfer-Encoding");
				if (it != json_request.end() && (*it).second == "chunked")
					chunked = 1;
			}
		}
//READ_SOCKET__________________________________________________________________
		bool	readSocketCheckEndRead()
		{
			if (chunked == 0 && request.size() == readByte)
			{
				readByte = 0;
				responseHeader.clear();

				if (json_request.find("Content-Length") != json_request.end())
					json_request["body"] = request;

				debug_info(getRequestMethod() + " " + getRequestHost()
						+ " " + getRequestTarget() + " BODY_SIZE:"
					+ std::to_string(json_request["body"].size()));

				/*
				std::cout << "____REQUEST_HEADER________\n";
				debug_show_map(json_request);
				std::cout << "BODY SIZE: "
					+ std::to_string(json_request["body"].size()) + '\n';
				std::cout << "__________________________\n";
				*/

				return (true);
			}
			return (false);
		}
//READ_SOCKET__________________________________________________________________
		void	exec_chunk()
		{
			size_t		found;

			while (chunked > 0)
			{
				if (chunked == 1)
				{
					found = request.find("\r\n");
					if (found == std::string::npos)
						break ;
					readByte = convert_str_to_base16(request.substr(0, found));
					chunked = 2;
					request.erase(0, found + 2);
				}

				if (chunked == 2)
				{
					if (request.size() < readByte + 2)
						break ;
					json_request["body"].append(request.substr(0, readByte));
					request.erase(0, readByte + 2);

					if (readByte == 0)
						chunked = 0;
					else
						chunked = 1;
				}
			}
		}
//FIND_LOCATION________________________________________________________________
		int		find_location()
		{
			location = nullptr;

			find_location_directory();

			if (location != nullptr)
			{
				if (location->getRule() != "/")
					path_file.erase(0, location->getRule().size());

				path_file.insert(0, location->getRoot());

				int	d = check_dir_or_file(path_file);
				if (d == 1)
				{
					if (path_file[path_file.size() - 1] != '/')
					{
						path_file.push_back('/');
						setResponseHeaderLocation(getRequestTarget() + "/");
					}

					d = check_dir_or_file(path_file + location->getIndex());

					if (d == -1 && !location->getAutoindex())
						path_file.append(location->getIndex());
					if (d == 0)
						path_file.append(location->getIndex());
				}
				find_location_filename_extension();
			}
			if (location == nullptr)
				return (1);
			return (0);
		}
//FIND_LOCATION________________________________________________________________
		int			find_location_directory()
		{
			std::string		request_target;
			size_t			found;

			request_target = path_file;
			while (request_target.length() != 0)
			{
				setLocation(server->findLocationRule(request_target));
				if (location != nullptr)
					return (0);
				if (request_target == "/")
					return (0);
				found = request_target.find_last_of("/");
				if (std::string::npos == found)
					return (0);
				if (found == 0)
					found = 1;
				request_target.erase(found);
			}
			return (0);
		}
//FIND_LOCATION________________________________________________________________
		void		find_location_filename_extension()
		{
			std::string		request_target;
			Location*		tmp_loc;

			size_t	found = path_file.find_last_of('.');
			if (found != std::string::npos)
			{
				request_target = path_file.substr(found);
				tmp_loc = server->findLocationRule(request_target);
				if (tmp_loc != nullptr)
					setLocation(tmp_loc);
			}
		}
//CHECK_REDIRECT_______________________________________________________________
		int		check_redirect()
		{
			if (location->getReturnCode() != 0)
				setResponseHeaderLocation(location->getReturnLocation());
			return (location->getReturnCode());
		}
//CHECK_BODY_SIZE______________________________________________________________
		int		check_413()
		{
			if (-1 == location->getClientMaxBodySize())
				return (0);
			if (json_request["body"].size() > location->getClientMaxBodySize())
				return (1);
			return (0);
		}
//CHECK_METHODS________________________________________________________________
		bool		check_501()
		{
			std::string	method = json_request["method"];

			if (method != "GET" && method != "POST" && method != "DELETE" && method != "PUT" && method != "HEAD")
			{
				responseHeader["Allow"] = "GET, POST, DELETE";
				return (true);
			}
			return (false);
		}
//CHECK_METHOD_________________________________________________________________
		bool		check_405(void)
		{
			std::string		method = json_request["method"];

			if (location->checkAccessMethod(method))
				return (false);
			return (true);
		}
//GET__________________________________________________________________________
		int		get_run()
		{
			DIR									*dp;
			struct dirent						*ep;
			std::vector<std::string>			name_file;

			if (check_dir_or_file(path_file) == 1 && location->getAutoindex())
			{
				dp = opendir(path_file.c_str());
				if (nullptr != dp)
				{
					while (nullptr != (ep = readdir(dp)))
						name_file.push_back(ep->d_name);
					closedir(dp);

					if (-1 == autoindex_rename_file(name_file))
						return (500);

					autoindex_create_body(name_file);
					return (200);
				}
				return (404);
			}
			return (open_file(path_file));
		}
//GET__________________________________________________________________________
		int		open_file(const std::string& str)
		{
			int				length;
			char*			buffer;
			int				status_code(200);

			std::ifstream	is (str, std::ifstream::binary);

			if (is)
			{
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
					is.close();
					return (500);
				}

				is.read (buffer,length);
				if (is)
				{
					body.assign(buffer, length);
					responseHeader_content_type(str);
				}
				else
					status_code = 500;

				is.close();
				delete [] buffer;
			}
			else
				status_code = 404;

			return (status_code);
		}

		void		responseHeader_content_type(const std::string& str)
		{
			size_t		found;

			if (location->rule == "/dowloads")
				responseHeader["Content-Type"] = "application/octet-stream";
			else
			{
				found = str.find_last_of('.');
				responseHeader["Content-Type"] = "text/plain";
				if (found != std::string::npos)
				{
					if (str.substr(found) == ".html")
						responseHeader["Content-Type"] = "text/html";
					else if (str.substr(found) == ".css")
						responseHeader["Content-Type"] = "text/css";
					else if (str.substr(found) == ".csv")
						responseHeader["Content-Type"] = "text/csv";
					else if (str.substr(found) == ".xml")
						responseHeader["Content-Type"] = "text/xml";
					else if (str.substr(found) == ".png")
						responseHeader["Content-Type"] = "image/png";
					else if (str.substr(found) == ".jpeg")
						responseHeader["Content-Type"] = "image/jpeg";
					else if (str.substr(found) == ".gif")
						responseHeader["Content-Type"] = "image/gif";
				}
			}
		}
//POST_________________________________________________________________________
		int				post_run()
		{
			if (check_dir_or_file(path_file) == 1)
				return (404);

			if (json_request["body"].size() == 0)
				return (204);

			std::ofstream	myfile(path_file, std::ios::binary);

			if (!myfile)
			{
				print_error(path_file + " no open");
				return (500);
			}
			myfile << json_request["body"];
			myfile.close();
			return (201);
		}
//DELETE_______________________________________________________________________
		int		deleteFile() const
		{
			if (unlink(path_file.c_str()) == -1)
				return 404;

			debug_info(path_file + " was deleted");
			return 204;
		}
//AUTOINDEX____________________________________________________________________
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
//AUTOINDEX____________________________________________________________________
		void	autoindex_create_body(std::vector<std::string>& name_file)
		{
			std::vector<std::string>::iterator	it;
			std::string							dir = getRequestTarget();

			body.assign("<html><head>\r\n");
			body.append("<meta http-equiv=\"Content-Type\" ");
			body.append("content=\"text/html;charset=utf-8\">\r\n");
			body.append("<title>Directory listing for " + dir + "</title>\r\n");
			body.append("</head>\r\n");
			body.append("<body>\r\n");
			body.append("<h1>Directory listing for " + dir + "</h1>\r\n");
			body.append("<hr>\r\n");
			body.append("<ul>\r\n");

			it = name_file.begin() + 2;
			for (; it != name_file.end(); ++it)
				body.append("<li><a href=\""+*it+"\">"+*it+"</a></li>\r\n");

			body.append("</ul>\r\n");
			body.append("<hr>\r\n");
			body.append("</body></html>\r\n");

			responseHeader["Content-Type"] = "text/html";
		}
//CGI__________________________________________________________________________
		int		cgi_run()
		{
			if (path_file[0] != '/')
				path_file = "/" + path_file;
			path_file = getenv("PWD") + path_file;

			char	**arg = cgi_create_arg();
			char	**env = cgi_create_env();
			int		status_code(500);

			/*
			std::cout << "___CGI_ARG________________________________\n";
			debug_show_arg(arg);
			std::cout << "___CGI_ENV________________________________\n";
			debug_show_map(envCgi);
			std::cout << "__________________________________________\n";
			*/

			if (arg && env)
				status_code = cgi_fork(arg, env);

			free_env(arg);
			free_env(env);

			return (status_code);
		}

//CGI__________________________________________________________________________
		void	cgi_return_save_std_fd(const int& in, const int& out)
		{
			dup2(in, 0);
			dup2(out, 1);

			close(in);
			close(out);
		}

		int		cgi_write(int fd_in)
		{
			std::string		tmp;

			if (json_request["body"].size() > 16384)
			{
				tmp = json_request["body"].substr(0, 16384);
				if (-1 == cgi_write_add(fd_in, tmp))
					return (-1);
			}

			else if (!json_request["body"].empty())
			{
				tmp = json_request["body"].substr();
				if (-1 == cgi_write_add(fd_in, tmp))
					return (-1);
			}

			if (json_request["body"].empty())
			{
				long bytes = write(fd_in, "\0", 1);
				if (bytes > 0)
					return (0);
				if (bytes < 0)
					return (-1);
			}

			return (1);
		}

		int		cgi_write_add(const int& fd_in, const std::string& tmp)
		{
			long bytes = write(fd_in, tmp.c_str(), tmp.size());

			if (bytes > 0)
				json_request["body"].erase(0, bytes);
			if (bytes < 0)
				return (-1);
			return (0);
		}

		int		cgi_read(const int& fd_out, int& status_code, int& f_header)
		{
			int			bytes(0);
			char		bu[20000];
			int			ofset;
			size_t		found;
			std::string	tmp;

			bytes = read(fd_out, bu, 20000);
			if (bytes > 0)
			{
				if (f_header == 0)
				{
					body.append(bu, bytes);

					ofset = 4;
					found = body.find("\r\n\r\n");
					if (found == std::string::npos)
					{
						ofset = 2;
						found = body.find("\n\n");
					}
					if (found != std::string::npos)
					{
						status_code = cgi_parser_header(body.substr(0, found));
						body.erase(0, found + ofset);

						bytes = body.size();
						f_header = 1;

						if (check_chunk_header_request() && bytes > 0)
						{
							tmp.assign(body);
							body.assign(convert_base16_to_str(body.size()));
							body.append("\r\n");
							body.append(tmp);
							body.append("\r\n");
						}
					}
				}
				else if (check_chunk_header_request())
				{
					tmp.assign(bu, bytes);

					body.append(convert_base16_to_str(bytes));
					body.append("\r\n");
					body.append(tmp);
					body.append("\r\n");
				}
				else
				{
					body.append(bu, bytes);
				}
			}

			if (bytes < 0)
				return (-1);

			if (bytes < 20000 && json_request["body"].size() == 0)
			{
				if (check_chunk_header_request())
					body.append("0\r\n\r\n");
				return (0);
			}

			return (1);
		}

		int		processing_write_read(const int& fd_in, const int& fd_out)
		{
			int			f_w(1);
			int			f_r(1);
			int			status_code(200);
			int			f_header(0);

			while (f_r)
			{
				if (f_w)
					f_w = cgi_write(fd_in);
				if (f_w < 0)
					return (500);

				f_r = cgi_read(fd_out, status_code, f_header);
				if (f_r < 0)
					return (500);
			}

			return (status_code);
		}
//CGI__________________________________________________________________________
		int		cgi_fork(char** arg, char** env)
		{
			int		fd_in_pipe[2], fd_out_pipe[2];
			int		status;
			int		status_code(200);
			pid_t	pid;

			if (pipe(fd_in_pipe) != 0 || pipe(fd_out_pipe) != 0)
			{
				print_error("CGI: pipe");
				return (500);
			}

			int save_fd_in = dup(0);
			int save_fd_out = dup(1);

			if ((dup2(fd_out_pipe[1], 1) == -1)
					|| (dup2(fd_in_pipe[0], 0) == -1))
			{
				print_error("CGI: dup2");
				return (500);
			}

			close(fd_in_pipe[0]);
			close(fd_out_pipe[1]);

			//запускаем дочерний процесс_______________________________________
			pid = fork();
			if (pid == -1)
			{
				close(fd_in_pipe[1]);
				close(fd_out_pipe[0]);
				cgi_return_save_std_fd(save_fd_in, save_fd_out);
				print_error("fork");
				return (500);
			}
			if (pid == 0)
			{
				close(fd_in_pipe[1]);
				close(fd_out_pipe[0]);
				execve(arg[0], arg, env);
				exit (errno);
			}

			cgi_return_save_std_fd(save_fd_in, save_fd_out);
			status_code = processing_write_read(fd_in_pipe[1], fd_out_pipe[0]);

			close(fd_in_pipe[1]);
			close(fd_out_pipe[0]);

			readByte = 0;
			wait(&status);

			if (status_code == 200 && check_chunk_header_request())
				responseHeader["Transfer-Encoding"] = "chunked";

			return (status_code);
		}

		int		check_chunk_header_request()
		{
			std::map<std::string, std::string>::iterator	it;

			it = json_request.find("Transfer-Encoding");
			if (it != json_request.end() && (*it).second == "chunked")
				return (1);
			return (0);
		}

		void	print_status_wait(const int& status)
		{
			std::cout << CYAN;

			if (WIFEXITED(status) != 0)
				std::cout << "CHILD correct end program" << std::endl;
			else
				std::cout << "CHILD no correct end program: " << WEXITSTATUS(status) << std::endl;

			if (WIFSIGNALED(status))
				std::cout << "CHILD call signal: " << WTERMSIG(status) << ": " << strsignal(WTERMSIG(status)) << std::endl;

			if (WIFSTOPPED(status))
				std::cout << "CHILD stop on signal " << WSTOPSIG(status) << std::endl;

			std::cout << RESET;
		}
//CGI__________________________________________________________________________
		char**	cgi_create_arg() const
		{
			char	**arg;

			try
			{
				arg = new char*[3];

				arg[0] = strdup(location->cgiPass.c_str());
				if (arg[0] == nullptr)
					return (free_env(arg));
				arg[1] = strdup(path_file.c_str());
				if (arg[1] == nullptr)
					return (free_env(arg));
				arg[2] = nullptr;
			}
			catch (std::bad_alloc& ba)
			{
				print_error("cgi_create_arg: bad_alloc");
				return (nullptr);
			}

			return (arg);
		}
//CGI__________________________________________________________________________
		char**	cgi_create_env(void)
		{
			print_debug("F create env");
			try
			{
				std::map<std::string, std::string>::iterator	it;
				int												i;
				char											**env;

				cgi_env();
				env = new char*[envCgi.size() + 1];

				i = 0;
				for (it = envCgi.begin(); it != envCgi.end(); ++it)
				{
					env[i] = strdup(((*it).first + "=" + (*it).second).c_str());
					if (env[i] == nullptr)
						return (free_env(env));
					++i;
				}
				env[i] = nullptr;

				return (env);
			}
			catch (std::bad_alloc& ba)
			{
				print_error("cgi_create_env: bad_alloc");
				return (nullptr);
			}
		}
//CGI__________________________________________________________________________
		void	cgi_env(void)
		{
			print_debug("F cgi env");

			//envCgi["AUTH_TYPE"] = "basic";							//delete
			envCgi["GATEWAY_INTERFACE"] = "CGI/0.1";
			envCgi["REMOTE_ADDR"] = addr;

			envCgi["SERVER_NAME"] = server->serverName;
			envCgi["SERVER_PORT"] = server->port;
			envCgi["SERVER_PROTOCOL"] = "HTTP/1.1";
			envCgi["SERVER_SOFTWARE"] = "La Femme Nikita 0.1";

			envCgi["REQUEST_METHOD"] = json_request["method"];
			envCgi["CONTENT_LENGTH"] = std::to_string(json_request["body"].size());
			envCgi["CONTENT_TYPE"] = json_request["Content-Type"];

			envCgi["PATH_INFO"] = "/Users/hyoghurt/ft_webserver/cgi_tester";
			envCgi["PATH_TRANSLATED"] = "";

			std::map<std::string, std::string>::iterator	it;
			it = json_request.find("X-Secret-Header-For-Test");
			if (it != json_request.end())
				envCgi["HTTP_X-Secret-Header-For-Test"] = (*it).second;

			/*
			//envCgi["REMOTE_HOST"] = "";		//delete
			//envCgi["REMOTE_IDENT"] = "";	//delete
			//envCgi["REMOTE_USER"] = "";		//delete
			*/
			//envCgi["SCRIPT_NAME"] = "./test.bla";
		}
//CGI__________________________________________________________________________
		int		cgi_parser_header(std::string str)
		{
			print_debug("cgi parser header");

			size_t									start(0);
			size_t									end(0);
			std::string								tmp;
			int										status_code(200);
			std::pair<std::string, std::string>		pr;

			while (1)
			{
				start = str.find_first_not_of("\r\n", end);
				if (start == std::string::npos)
					return (status_code);

				end = str.find("\r\n", start);
				if (end == std::string::npos)
					end = str.find("\n", start);

				if (end == std::string::npos)
					tmp = str.substr(start);
				else
					tmp = str.substr(start, end - start);

				if (!tmp.empty())
				{
					pr = return_key_val(tmp);
					if (pr.first == "Status")
						status_code = atoi(pr.second.substr(0, 3).c_str());
					else
						responseHeader[pr.first] = pr.second;
				}
			}
			return (status_code);
		}
//CGI__________________________________________________________________________
		std::map<std::string, std::string>
			cgi_parser_header_one(const std::string& str)
		{
			std::map<std::string, std::string>	m;
			size_t								end;
			size_t								start(0);
			std::string							tmp;

			start = str.find_first_not_of(" ", start);
			if (start == std::string::npos)
				return (m);

			end = str.find(" ", start);
			if (end == std::string::npos)
			{
				m["http_version"] = str.substr(start);
				return (m);
			}
			m["http_version"] = str.substr(start, end - start);

			start = str.find_first_not_of(" ", end);
			if (end == std::string::npos)
			{
				m.clear();
				return (m);
			}
			m["status code"] = str.substr(start);
			return (m);
		}
//RESPONSE_HEADER______________________________________________________________
		void	response_header_connection(void)
		{
			responseHeader["Connection"] = json_request["Connection"];
			if (responseHeader["Connection"] == "close")
				return ;
			responseHeader["Connection"] = "Keep-Alive";
			responseHeader["Keep-Alive"] = "timeout="
					+ std::to_string(TIME_KEEP_ALIVE);
		}
//RESPONSE_HEADER______________________________________________________________
		void	response_total(void)
		{
			std::map<std::string, std::string>::iterator	it;

			responseHeader["Host"] = json_request["Host"];

			response_header_connection();

			if (!body.empty() && !check_chunk_header_request())
				responseHeader["Content-Length"] = std::to_string(body.size());

			header = "HTTP/1.1 "
				+ responseHeader["Status"] + "\r\n";
			header += "Host: " + responseHeader["Host"] + "\r\n";

			for (it = responseHeader.begin(); it != responseHeader.end(); ++it)
			{
				if ((*it).first == "Status")
					continue;
				if ((*it).first == "Host")
					continue;
				if (!(*it).second.empty())
					header += (*it).first + ": " + (*it).second + "\r\n";
			}
			header += "\r\n";

			if (json_request["method"] == "HEAD")
				response = header;
			else
				response = header + body;

			body.clear();
			header.clear();
			json_request.clear();
		}
//ADD__________________________________________________________________________
		void	debug_info(const std::string& mes) const
		{
			std::cout << YELLOW << get_new_time() << " Cl:"
			<< socket << " " << mes <<  RESET << '\n';
		}

		static char**	free_env(char** env)
		{
			print_debug("F free env");
			int		i = 0;

            if (!env)
                return (nullptr);
			while (env[i])
				free(env[i++]);
			delete [] env;
			return (nullptr);
		}

		void	debug_show_map(std::map<std::string, std::string>& m)
		{
			std::map<std::string, std::string>::iterator	it;

			for (it = m.begin(); it != m.end(); ++it)
			{
				if ((*it).first == "body")
					continue ;
				std::cout << (*it).first << "=" << (*it).second << std::endl;
			}
		}

		void	debug_show_arg(char** env)
		{
			int		i = 0;

			while (env[i] != nullptr)
				std::cout << env[i++] << std::endl;
		}

		void	find_query_string_path_info()
		{
			size_t	found;

			path_file = getRequestTarget();

			envCgi["QUERY_STRING"] = "/";
			found = path_file.find('?');
			if (std::string::npos != found)
			{
				envCgi["QUERY_STRING"] = path_file.substr(found + 1);
				path_file.erase(found);
			}

			found = path_file.find_last_of('.');
			if (std::string::npos != found)
			{
				found = path_file.find('/', found);
				if (std::string::npos != found)
				{
					envCgi["PATH_INFO"] = path_file.substr(found);
					path_file.erase(found);
				}
			}
		}

		std::pair<std::string, std::string>
			return_key_val(const std::string& str)
		{
			int				start(0);
			int				end(0);
			std::string		key("");
			std::string		val("");

			start = str.find_first_not_of(" ", start);
			end = str.find(":");
			if (end != std::string::npos)
			{
				key = str.substr(start, end - start);
				start = str.find_first_not_of(" ", end + 1);
				val = str.substr(start);
			}

			return (std::pair<std::string, std::string>(key, val));
		}

		void	create_json_request_header(const std::string& str)
		{
			size_t			end;
			size_t			start(0);
			std::string		tmp;

			end = str.find(" ", start);
			if (end == std::string::npos)
				return ;
			json_request["method"] = str.substr(start, end - start);

			start = end + 1;
			end = str.find(" ", start);
			if (end == std::string::npos)
				return ;
			json_request["request_target"] = str.substr(start, end - start);

			start = end + 1;
			end = str.find("\r", start);
			if (end == std::string::npos)
				return ;
			json_request["http_version"] = str.substr(start, end - start);

			start = end + 2;
			end = str.find("\n", start);
			if (end == std::string::npos)
				return ;
			while (end != std::string::npos)
			{
				tmp = str.substr(start, end - start - 1);
		
				json_request.insert(return_key_val(tmp));
		
				start = end + 1;
				end = str.find("\n", start);
			}
			tmp = str.substr(start);
			json_request.insert(return_key_val(tmp));
		}

		/*
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
		*/

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
		std::string							addr;
		int									flag;
		int									chunked;
};

#endif
