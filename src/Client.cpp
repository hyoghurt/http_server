#include "Client.hpp"

Client::Client(const int& cSocket, struct sockaddr_in addr) : cSocket(cSocket), addr(addr)
{
	time(&timeStart);
	request = "";
	response = "";
	readByte = 0;
	fWrite = 0;
	chunked = 0;
}

Client::Client(const Client& oth)
{ *this = oth; }

Client::~Client()
{}


Client&		Client::operator= (const Client& oth)
{
	this->cSocket = oth.cSocket;
	this->timeStart = oth.timeStart;
	this->request = oth.request;
	this->response = oth.response;
	this->header = oth.header;
	this->body = oth.body;
	this->readByte = oth.readByte;
	this->jsonRequest = oth.jsonRequest;
	this->server = oth.server;
	this->location = oth.location;
	this->pathFile = oth.pathFile;
	this->responseHeader = oth.responseHeader;
	this->envCgi = oth.envCgi;
	this->addr = oth.addr;
	this->fWrite = oth.fWrite;
	this->chunked = oth.chunked;
	return *this;
}

const int&			Client::getSocket() const
{ return this->cSocket; }

const time_t&		Client::getTimeStart(void) const
{ return this->timeStart; }

const std::string&	Client::getRequest(void) const
{ return this->request; }

const std::string&	Client::getResponse(void) const
{ return this->response; }

const std::string&	Client::getHeader(void) const
{ return this->header; }

const std::string&	Client::getBody(void) const
{ return this->body; }

const int&			Client::getReadByte() const
{ return this->readByte; }

const std::string	Client::getRequestTarget()
{
	std::map<std::string, std::string>::iterator	it;

	it = jsonRequest.find("request_target");
	if (it != jsonRequest.end())
		return (*it).second;
	return ("");
}

const std::string	Client::getRequestMethod()
{
	std::map<std::string, std::string>::iterator	it;

	it = jsonRequest.find("method");
	if (it != jsonRequest.end())
		return (*it).second;
	return ("");
}

const std::string	Client::getRequestHost(void)
{
	std::map<std::string, std::string>::iterator	it;

	it = jsonRequest.find("Host");
	if (it != jsonRequest.end())
		return (*it).second;
	return ("");
}

const Server*		Client::getServer() const
{ return this->server; }

const Location*		Client::getLocation() const
{ return this->location; }

const std::string	Client::getPathErrorPage(const int& code) const
{
	if (server == nullptr)
		return ("");
	return (server->getPathErrorPage(code));
}

const std::string&	Client::getPathFile() const
{ return this->pathFile; }

const int&			Client::getChunked() const
{ return this->chunked; }

const bool&			Client::getFWrite() const
{ return this->fWrite; }


void				Client::setSocket(const int &cSocket)
{ this->cSocket = cSocket; }

void				Client::setTimeStart(void)
{ time(&timeStart); }

void				Client::setRequest(const std::string &request)
{ this->request = request; }

void				Client::setRequestAppend(const std::string& str)
{ this->request.append(str); }

void				Client::setRequestAppend(const char* buf, size_t len)
{ this->request.append(buf, len); }

void				Client::setResponse(const std::string &response)
{ this->response = response; }

void				Client::setServer(Server* serv)
{ this->server = serv; }

void				Client::setLocation(Location* loc)
{ this->location = loc; }

void				Client::setPathFile(const std::string& str)
{ this->pathFile = str; }

void				Client::setResponseHeaderStatus(const int& code)
{ responseHeader["Status"] = get_status_code(code); }

void				Client::setResponseHeaderLocation(const std::string& str)
{ responseHeader["Location"] = str; }

bool				Client::checkResponseHeaderConnectionClose()
{
	std::map<std::string, std::string>::iterator	it;

	it = responseHeader.find("Connection");
	if (it != responseHeader.end() && (*it).second == "close")
		return (true);
	return (false);
}

void				Client::upFWrite()
{ this->fWrite = true; }

void				Client::downFWrite()
{ this->fWrite = false; }
//READ_SOCKET__________________________________________________________________
void				Client::readEraseHeaderRequest()
{
	std::map<std::string, std::string>::iterator	it;

	size_t	found = request.find("\r\n\r\n");

	if (found != std::string::npos)
	{
		createJsonRequestHeader(request.substr(0, found));
		request.erase(0, found + 4);

		it = jsonRequest.find("Content-Length");
		if (it != jsonRequest.end())
			readByte = atoi((*it).second.c_str());

		it = jsonRequest.find("Transfer-Encoding");
		if (it != jsonRequest.end() && (*it).second == "chunked")
			chunked = 1;
	}
}

bool				Client::readSocketCheckEndRead()
{
	if (chunked == 0 && request.size() == readByte)
	{
		readByte = 0;
		responseHeader.clear();

		if (jsonRequest.find("Content-Length") != jsonRequest.end())
			jsonRequest["body"] = request;

		debugInf(getRequestMethod() + " " + getRequestHost()
				+ " " + getRequestTarget() + " BODY_SIZE:"
			+ std::to_string(jsonRequest["body"].size()));

		/*
		std::cout << "____REQUEST_HEADER________\n";
		debug_show_map(jsonRequest);
		std::cout << "BODY SIZE: "
			+ std::to_string(jsonRequest["body"].size()) + '\n';
		std::cout << "__________________________\n";
		*/

		return (true);
	}
	return (false);
}

void				Client::readExecChunk()
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
			jsonRequest["body"].append(request.substr(0, readByte));
			request.erase(0, readByte + 2);

			if (readByte == 0)
				chunked = 0;
			else
				chunked = 1;
		}
	}
}
//WRITE_SOCKET_________________________________________________________________
int				Client::writeSocket()
{
	int		bytes(0);

	if (fWrite)
	{
		setTimeStart();

		bytes = write(getSocket(), response.c_str(),
				response.size());

		if (bytes > 0)
		{
			response.erase(0, bytes);
			if (response.empty())
			{
				downFWrite();
				debugInf(responseHeader["Status"]);
			}
		}
	}
	return (bytes);
}
//FIND_LOCATION________________________________________________________________
int				Client::locationFind()
{
	location = nullptr;

	locationFindDirectory();

	if (location != nullptr)
	{
		if (location->getRule() != "/")
			pathFile.erase(0, location->getRule().size());

		pathFile.insert(0, location->getRoot());
		envCgi["PATH_INFO"] = getenv("PWD") + std::string("/")
			+ location->getRoot() + envCgi["PATH_INFO"];

		int	d = check_dir_or_file(pathFile);
		if (d == 1)
		{
			if (pathFile[pathFile.size() - 1] != '/')
			{
				pathFile.push_back('/');
				setResponseHeaderLocation(getRequestTarget() + "/");
			}

			d = check_dir_or_file(pathFile + location->getIndex());

			if (d == -1 && !location->getAutoindex())
				pathFile.append(location->getIndex());
			if (d == 0)
				pathFile.append(location->getIndex());
		}
		locationFindExtension();
	}
	if (location == nullptr)
		return (1);
	return (0);
}

int			Client::locationFindDirectory()
{
	std::string		request_target;
	size_t			found;

	request_target = pathFile;
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

void		Client::locationFindExtension()
{
	std::string		request_target;
	Location*		tmp_loc;

	size_t	found = pathFile.find_last_of('.');
	if (found != std::string::npos)
	{
		request_target = pathFile.substr(found);
		tmp_loc = server->findLocationRule(request_target);
		if (tmp_loc != nullptr)
			setLocation(tmp_loc);
	}
}

//CHECK_REDIRECT_______________________________________________________________
int			Client::check301()
{
	if (location->getReturnCode() != 0)
		setResponseHeaderLocation(location->getReturnLocation());
	return (location->getReturnCode());
}

//RESPONSE_____________________________________________________________________
int			Client::check413()
{
	if (-1 == location->getClientMaxBodySize())
		return (0);
	if (jsonRequest["body"].size() > location->getClientMaxBodySize())
		return (1);
	return (0);
}

bool		Client::check501()
{
	std::string	method = jsonRequest["method"];

	if (method != "GET" && method != "POST" && method != "DELETE"
			&& method != "PUT" && method != "HEAD")
	{
		responseHeader["Allow"] = "GET, POST, DELETE";
		return (true);
	}
	return (false);
}

bool		Client::check405(void)
{
	std::string		method = jsonRequest["method"];

	if (location->checkAccessMethod(method))
		return (false);
	return (true);
}

//GET__________________________________________________________________________
int			Client::runGet()
{
	DIR									*dp;
	struct dirent						*ep;
	std::vector<std::string>			name_file;

	if (check_dir_or_file(pathFile) == 1 && location->getAutoindex())
	{
		dp = opendir(pathFile.c_str());
		if (nullptr != dp)
		{
			while (nullptr != (ep = readdir(dp)))
				name_file.push_back(ep->d_name);
			closedir(dp);

			if (-1 == autoindexRenameFile(name_file))
				return (500);

			autoindexCreateBody(name_file);
			return (200);
		}
		return (404);
	}
	return (getOpenFile(pathFile));
}

int			Client::getOpenFile(const std::string& str)
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
			print_error("getOpenFile: bad_alloc");
			is.close();
			return (500);
		}

		is.read (buffer,length);
		if (is)
		{
			body.assign(buffer, length);
			getResponseHeaderContentType(str);
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

void		Client::getResponseHeaderContentType(const std::string& str)
{
	size_t		found;

	if (location->getRule() == "/dowloads" && pathFile == str)
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
int				Client::runPost()
{
	if (check_dir_or_file(pathFile) == 1)
		return (404);

	if (jsonRequest["body"].size() == 0)
		return (204);

	std::ofstream	myfile(pathFile, std::ios::binary);

	if (!myfile)
	{
		print_error(pathFile + " no open");
		return (500);
	}
	myfile << jsonRequest["body"];
	myfile.close();
	return (201);
}

//DELETE_______________________________________________________________________
int			Client::runDelete()
{
	if (unlink(pathFile.c_str()) == -1)
		return 404;

	debugInf(pathFile + " was deleted");
	return 204;
}

//AUTOINDEX____________________________________________________________________
int			Client::autoindexRenameFile(std::vector<std::string>& name_file)
{
	std::vector<std::string>::iterator	it;
	int									ret;

	it = name_file.begin() + 2;

	for (; it != name_file.end(); ++it)
	{
		ret = check_dir_or_file(pathFile + *it);
		if (ret == 1)
			(*it).push_back('/');
		else if (ret == -1)
			return (-1);
	}
	return (0);
}

void		Client::autoindexCreateBody(std::vector<std::string>& name_file)
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
int			Client::runCgi()
{
	if (pathFile[0] != '/')
		pathFile = "/" + pathFile;
	pathFile = getenv("PWD") + pathFile;

	char	**arg = cgiCreateArg();
	char	**env = cgiCreateEnv();

	int		status_code(500);

	/*
	std::cout << "___CGI_ARG________________________________\n";
	debug_show_arg(arg);
	std::cout << "___CGI_ENV________________________________\n";
	debug_show_map(envCgi);
	std::cout << "__________________________________________\n";
	*/

	if (arg && env)
		status_code = cgiFork(arg, env);

	freeEnv(arg);
	freeEnv(env);

	return (status_code);
}

int			Client::cgiFork(char** arg, char** env)
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
		cgiRetrunSaveStdFd(save_fd_in, save_fd_out);
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

	cgiRetrunSaveStdFd(save_fd_in, save_fd_out);
	status_code = cgiProcessingWriteRead(pid, fd_in_pipe[1], fd_out_pipe[0]);

	close(fd_in_pipe[1]);
	close(fd_out_pipe[0]);

	wait(&status);

	if (WIFEXITED(status) != 0)
		if (WEXITSTATUS(status) == 2)
			status_code = 404;

	if (status_code == 200 && checkHeaderRequestChunk())
		responseHeader["Transfer-Encoding"] = "chunked";

	return (status_code);
}

int		Client::cgiProcessingWriteRead
			(const int& pid, const int& fd_in, const int& fd_out)
{
	int			f_w(1);
	int			f_r(1);
	int			status_code(200);
	int			f_header(0);
	int			status;
	int			w;

	while (f_r)
	{
		if (f_w)
			f_w = cgiWrite(fd_in);
		if (f_w < 0)
			return (500);

		f_r = cgiRead(fd_out, status_code, f_header);
		if (f_r < 0)
			return (500);
	}

	return (status_code);
}

int			Client::cgiWrite(int fd_in)
{
	std::string		tmp;
	size_t			bufSize(16384);

	if (jsonRequest["body"].size() > bufSize)
	{
		tmp = jsonRequest["body"].substr(0, bufSize);
		if (-1 == cgiWriteAdd(fd_in, tmp))
			return (-1);
	}
	else if (!jsonRequest["body"].empty())
	{
		tmp = jsonRequest["body"].substr();
		if (-1 == cgiWriteAdd(fd_in, tmp))
			return (-1);
	}
	if (jsonRequest["body"].empty())
	{
		long bytes = write(fd_in, "\0", 1);
		if (bytes > 0)
			return (0);
		if (bytes < 0)
			return (-1);
	}
	return (1);
}

int			Client::cgiWriteAdd(const int& fd_in, const std::string& tmp)
{
	int	bytes = write(fd_in, tmp.c_str(), tmp.size());

	if (bytes > 0)
		jsonRequest["body"].erase(0, bytes);
	if (bytes < 0)
		return (-1);
	return (0);
}

int		Client::cgiRead(const int& fd_out, int& status_code, int& f_header)
{
	int			bytes(0);
	std::string	tmp;
	size_t		bufSize(20000);
	char		bu[bufSize];

	bytes = read(fd_out, bu, bufSize);

	if (bytes > 0)
	{
		if (f_header == 0)
		{
			body.append(bu, bytes);
			f_header = cgiFindHeader(status_code);
		}
		else if (checkHeaderRequestChunk())
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

	if (bytes < bufSize && jsonRequest["body"].size() == 0)
	{
		if (checkHeaderRequestChunk())
			body.append("0\r\n\r\n");
		return (0);
	}

	return (1);
}

int			Client::cgiFindHeader(int& status_code)
{
	int			ofset(4);
	size_t		found;
	std::string	tmp;

	found = body.find("\r\n\r\n");
	if (found == std::string::npos)
	{
		ofset = 2;
		found = body.find("\n\n");
	}
	if (found != std::string::npos)
	{
		status_code = cgiParserHeader(body.substr(0, found));
		body.erase(0, found + ofset);

		if (checkHeaderRequestChunk() && body.size() > 0)
		{
			tmp.assign(body);
			body.assign(convert_base16_to_str(body.size()));
			body.append("\r\n");
			body.append(tmp);
			body.append("\r\n");
		}
		return (1);
	}
	return (0);
}

void		Client::cgiRetrunSaveStdFd(const int& in, const int& out)
{
	dup2(in, 0);
	dup2(out, 1);

	close(in);
	close(out);
}

char**		Client::cgiCreateArg()
{
	char	**arg;

	try
	{
		arg = new char*[3];

		arg[0] = strdup(location->getCgiPass().c_str());
		if (arg[0] == nullptr)
			return (freeEnv(arg));
		arg[1] = strdup(pathFile.c_str());
		if (arg[1] == nullptr)
			return (freeEnv(arg));
		arg[2] = nullptr;
	}
	catch (std::bad_alloc& ba)
	{
		print_error("cgiCreateArg: bad_alloc");
		return (nullptr);
	}

	return (arg);
}

char**		Client::cgiCreateEnv(void)
{
	try
	{
		std::map<std::string, std::string>::iterator	it;
		int												i;
		char											**env;

		cgiEnv();

		env = new char*[envCgi.size() + 1];

		i = 0;
		for (it = envCgi.begin(); it != envCgi.end(); ++it)
		{
			env[i] = strdup(((*it).first + "=" + (*it).second).c_str());
			if (env[i] == nullptr)
				return (freeEnv(env));
			++i;
		}
		env[i] = nullptr;

		return (env);
	}
	catch (std::bad_alloc& ba)
	{
		print_error("cgiCreateEnv: bad_alloc");
		return (nullptr);
	}
}

void		Client::cgiEnv()
{
	envCgi["SERVER_NAME"] = server->getServerName();
	envCgi["SERVER_PORT"] = server->getPort();
	envCgi["SERVER_PROTOCOL"] = "HTTP/1.1";
	envCgi["SERVER_SOFTWARE"] = "La Femme Nikita 0.1";
	envCgi["GATEWAY_INTERFACE"] = "CGI/0.1";
	envCgi["REQUEST_METHOD"] = jsonRequest["method"];
	envCgi["CONTENT_LENGTH"] = std::to_string(jsonRequest["body"].size());
	envCgi["CONTENT_TYPE"] = jsonRequest["Content-Type"];
	if (!checkCgiTester())
		envCgi["SCRIPT_NAME"] = jsonRequest["request_target"];
	envCgi["REMOTE_ADDR"] = inet_ntoa(addr.sin_addr);
	//envCgi["REMOTE_PORT"] = ntohs(addr.sin_port);

	cgiFurunkul();
}

void		Client::cgiFurunkul()
{
	std::map<std::string, std::string>::iterator	it;

	for (it = jsonRequest.begin(); it != jsonRequest.end(); ++it)
	{
		if ((*it).first == "method")
			continue ;
		if ((*it).first == "request_target")
			continue ;
		if ((*it).first == "http_version")
			continue ;
		if ((*it).first == "Host")
			continue ;
		if ((*it).first == "body")
			continue ;
		if ((*it).first == "Connection")
			continue ;
		if ((*it).first == "Content-Length")
			continue ;
		if ((*it).first == "Transfer-Encoding")
			continue ;
		envCgi["HTTP_" + (*it).first] = (*it).second;
	}
}

int			Client::cgiParserHeader(std::string str)
{
	size_t									start;
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
			pr = returnKeyVal(tmp);
			if (pr.first == "Status")
				status_code = atoi(pr.second.substr(0, 3).c_str());
			else
				responseHeader[pr.first] = pr.second;
		}
	}
	return (status_code);
}

//RESPONSE_HEADER______________________________________________________________
void		Client::responseTotal(void)
{
	std::map<std::string, std::string>::iterator	it;

	responseHeader["Host"] = jsonRequest["Host"];

	responseHeaderConnection();

	if (!body.empty() && !checkHeaderRequestChunk())
		responseHeader["Content-Length"] = std::to_string(body.size());

	header.assign("HTTP/1.1 " + responseHeader["Status"] + "\r\n");
	header.append("Host: " + server->getServerName() + "\r\n");

	for (it = responseHeader.begin(); it != responseHeader.end(); ++it)
	{
		if ((*it).first == "Status")
			continue;
		if ((*it).first == "Host")
			continue;
		if (!(*it).second.empty())
			header.append((*it).first + ": " + (*it).second + "\r\n");
	}
	header.append("\r\n");

	if (jsonRequest["method"] == "HEAD")
		response = header;
	else
		response = header + body;

	body.clear();
	header.clear();
	jsonRequest.clear();
	readByte = 0;
}

void		Client::responseHeaderConnection(void)
{
	responseHeader["Connection"] = jsonRequest["Connection"];
	if (responseHeader["Connection"] == "close")
		return ;
	responseHeader["Connection"] = "Keep-Alive";
	responseHeader["Keep-Alive"] = "timeout="
			+ std::to_string(TIME_KEEP_ALIVE);
}
//ADD__________________________________________________________________________
void		Client::debugInfConnect(int socket_listen)
{
	std::cout << GREEN << get_new_time() << " Cl:"
	<< cSocket << " " << inet_ntoa(addr.sin_addr) << ":"
	<< ntohs(addr.sin_port) << " connect socket listen: "
	<< socket_listen <<  RESET << '\n';
}

void		Client::debugInf(const std::string& mes) const
{
	std::cout << YELLOW << get_new_time() << " Cl:"
	<< cSocket << " " << mes <<  RESET << '\n';
}

void		Client::debugInfClose(const std::string& mes) const
{
	std::cout << CYAN << get_new_time() << " Cl:"
	<< cSocket << " " << mes <<  RESET << '\n';
}

bool		Client::checkHeaderRequestChunk()
{
	std::map<std::string, std::string>::iterator	it;

	it = jsonRequest.find("Transfer-Encoding");
	if (it != jsonRequest.end() && (*it).second == "chunked")
		return (true);
	return (false);
}

void		Client::findQueryStringPathInfo()
{
	size_t	found;

	envCgi.clear();
	pathFile = getRequestTarget();

	envCgi["QUERY_STRING"] = "";
	found = pathFile.find('?');
	if (std::string::npos != found)
	{
		envCgi["QUERY_STRING"] = pathFile.substr(found + 1);
		pathFile.erase(found);
	}

	found = pathFile.find_last_of('.');
	if (std::string::npos != found)
	{
		found = pathFile.find('/', found);
		if (std::string::npos != found)
		{
			envCgi["PATH_INFO"] = pathFile.substr(found);
			pathFile.erase(found);
		}
	}
}

bool		Client::checkCgiTester()
{
	std::string		tmp(location->getCgiPass());

	size_t found = tmp.find_last_of('/');
	if (found != std::string::npos)
		if (tmp.erase(0, found + 1) == "cgi_tester")
			return (true);
	return (false);
}

char**	Client::freeEnv(char** env)
{
	int		i = 0;

    if (!env)
        return (nullptr);
	while (env[i])
		free(env[i++]);
	delete [] env;
	return (nullptr);
}


void		Client::createJsonRequestHeader(const std::string& str)
{
	size_t			end;
	size_t			start(0);
	std::string		tmp;

	end = str.find(" ", start);
	if (end == std::string::npos)
		return ;
	jsonRequest["method"] = str.substr(start, end - start);

	start = end + 1;
	end = str.find(" ", start);
	if (end == std::string::npos)
		return ;
	jsonRequest["request_target"] = str.substr(start, end - start);

	start = end + 1;
	end = str.find("\r", start);
	if (end == std::string::npos)
		return ;
	jsonRequest["http_version"] = str.substr(start, end - start);

	start = end + 2;
	end = str.find("\n", start);
	if (end == std::string::npos)
		return ;
	while (end != std::string::npos)
	{
		tmp = str.substr(start, end - start - 1);
		jsonRequest.insert(returnKeyVal(tmp));
		start = end + 1;
		end = str.find("\n", start);
	}
	tmp = str.substr(start);
	jsonRequest.insert(returnKeyVal(tmp));
}

//DEBUG________________________________________________________________________
bool		Client::print_status_wait(const int& status)
{
	std::cout << CYAN;

	if (WIFEXITED(status) != 0)
		std::cout << "CHILD correct end program" << std::endl;
	else
	{
		std::cout << "CHILD no correct end program: " << WEXITSTATUS(status) << std::endl;
		std::cout << errno << std::endl;
		std::cout << "errno:" << strerror(errno) << '\n';
	}
	if (WIFSIGNALED(status))
		std::cout << "CHILD call signal: " << WTERMSIG(status) << ": " << strsignal(WTERMSIG(status)) << std::endl;
	if (WIFSTOPPED(status))
		std::cout << "CHILD stop on signal " << WSTOPSIG(status) << std::endl;

	std::cout << RESET;

	return (false);
}

void		Client::debug_show_map(std::map<std::string, std::string>& m)
{
	std::map<std::string, std::string>::iterator	it;

	for (it = m.begin(); it != m.end(); ++it)
	{
		if ((*it).first == "body")
			continue ;
		std::cout << (*it).first << "=" << (*it).second << std::endl;
	}
}

void		Client::debug_show_arg(char** env)
{
	int		i(0);

	while (env[i] != nullptr)
		std::cout << env[i++] << std::endl;
}
