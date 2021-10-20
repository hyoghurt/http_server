#ifndef CLIENT_HPP
# define CLIENT_HPP

# define BUF_SIZE 10000
# define TIME_KEEP_ALIVE 3

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
			flag = 0;
			chunked = 0;
			str_chunked = "";
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
			this->str_chunked = oth.str_chunked;
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

		int		check_501(void)
		{
			std::string	method;

			method = json_request["method"];

			if (method != "GET" && method != "POST" && method != "DELETE" && method != "PUT" && method != "HEAD")
			{
				responseHeader["Allow"] = "GET, POST, DELETE";
				return (1);
			}
			return (0);
		}

		int		check_redirect()
		{
			if (location->return_code != 0)
				responseHeader["Location"] = location->return_location;
			return (location->return_code);
		}

		int		check_413(void)
		{
			if (-1 == location->clientMaxBodySize)
				return (0);

			if (json_request["body"].size() > location->clientMaxBodySize)
				return (1);

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
//GET__________________________________________________________________________
		int		get_run()
		{
			print_debug("F method GET");

			DIR									*dp;
			struct dirent						*ep;
			std::vector<std::string>			name_file;

			if (check_dir_or_file(path_file) == 1 && location->autoindex)
			{
				print_debug("F autoindex true");

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
			print_debug("F open file: " + str);

			int				length;
			char*			buffer;
			int				status_code(200);

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
			print_debug("F post file: " + path_file);

			int	f = check_dir_or_file(path_file);
			if (f == 1)
				return (404);

			if (json_request["body"].size() == 0)
				return (204);

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
//DELETE_______________________________________________________________________
		int		deleteFile() const
		{
			print_debug("F delete file: " + path_file);

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
			std::string&		dir = json_request["request_target"];

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
			print_debug("F run cgi");

			/*
			int		f = check_dir_or_file(path_file);
			if (f == -1)
				return (404);
			*/

			body.clear();
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
//CGI__________________________________________________________________________
		int		cgi_fork(char** arg, char** env)
		{
			print_debug("F cgi fork");

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

			long		bytes(0);
			std::string	tmp;
			int			w(0);
			char		bu[20001];

			int			f_w(1);
			int			f_r(1);
			int			found;
			int			ofset;

			readByte = 0;

			while (f_r)
			{
				if (f_w)
				{
					if (json_request["body"].size() > 16384)
					{
						tmp = json_request["body"].substr(0, 16384);
						bytes = write(fd_in_pipe[1], tmp.c_str(), tmp.size());
						if (bytes > 0)
						{
							json_request["body"].erase(0, bytes);
						}
					}
					else if (!json_request["body"].empty())
					{
						tmp = json_request["body"].substr();
						bytes = write(fd_in_pipe[1], tmp.c_str(), tmp.size());
						if (bytes > 0)
						{
							json_request["body"].erase(0, bytes);
						}
					}

					if (json_request["body"].empty())
					{
						bytes = write(fd_in_pipe[1], "\0", 1);
						if (bytes > 0)
						{
							f_w = 0;
						}
					}
				}

				bytes = read(fd_out_pipe[0], bu, 20000);
				if (bytes > 0)
				{
					if (readByte == 0)
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
							readByte = 1;
							//strcpy (bu, body.c_str());

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

				if (bytes < 20000 && json_request["body"].size() == 0)
				{
					if (check_chunk_header_request())
						body.append("0\r\n\r\n");
					f_r = 0;
				}
			}

			close(fd_in_pipe[1]);
			close(fd_out_pipe[0]);

			readByte = 0;

			wait(&status);

				//std::cout << "bytes read: " << bytes << std::endl;
				/*
				w = waitpid(0, &status, WNOHANG);
				std::cout << "w=" << w << std::endl;

				print_status_wait(status);

				if (WIFEXITED(status) != 0)
					break ;

				if (w == 0)
					break ;

				if (w == -1)
				{
					print_error("waitpid -1");
					break ;
				}
				*/

			/*
			wirte exit status code this no work
			std::cout << "exit STATUS " << WEXITSTATUS(status) << std::endl;
			std::cout << "exit STATUS " << status << std::endl;
			if (WEXITSTATUS(status) == 2)
			{
				status_code = 404;
				print_info("execve exit 2");
			}
			else if (WEXITSTATUS(status) != 0)
			{
				status_code = 500;
				print_error("execve exit 0");
			}
			*/

			std::cout << "status_code= " << status_code << std::endl;
			if (status_code == 200 && check_chunk_header_request())
				responseHeader["Transfer-Encoding"] = "chunked";
			std::cout << "BODY_SIZE=" << body.size() << std::endl;

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
		int		cgi_read(int fd)
		{
			long    bytes;

			body.clear();

			bytes = read(fd, buf, BUF_SIZE - 1);
			while (bytes > 0)
			{
				buf[bytes] = '\0';
				body.append(buf);
				bytes = read(fd, buf, BUF_SIZE - 1);
			}

			if (bytes < 0)
				return (500);
			return (200);
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
			path_file = json_request["request_target"];

			envCgi["QUERY_STRING"] = "/";
			int	found = path_file.find('?');
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
		std::string							str_chunked;
};

#endif
