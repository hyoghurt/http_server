#ifndef CLIENT_HPP
# define CLIENT_HPP

# define BUF_SIZE 10000
# define TIME_KEEP_ALIVE 3

# include <dirent.h> //opendir
# include <arpa/inet.h> //sockaddr_in
# include <fstream> //open, ifstream
# include <unistd.h> //close

# include "addFunctions.hpp"
# include "Server.hpp"

class	Client
{
	public:
		Client(const int& cSocket, struct sockaddr_in addr);
		Client(const Client& oth);
		~Client();

		Client&				operator= (const Client& oth);
//GET_SET______________________________________________________________________
		const int&			getSocket() const;
		const time_t&		getTimeStart(void) const;
		const std::string&	getRequest() const;
		const std::string& 	getResponse() const;
		const std::string& 	getHeader() const;
		const std::string& 	getBody() const;
		const int&			getReadByte() const;
		const std::string	getRequestTarget();
		const std::string	getRequestMethod();
		const std::string	getRequestHost();
		const Server*		getServer() const;
		const Location*		getLocation() const;
		const std::string	getPathErrorPage(const int& code) const;
		const std::string&	getPathFile() const;
		const int&			getChunked() const;
		const bool&			getFWrite() const;

		void	setSocket(const int &cSocket);
		void	setTimeStart();
		void	setRequest(const std::string &request);
		void	setRequestAppend(const std::string& str);
		void	setRequestAppend(const char* buf, size_t len);
		void	setResponse(const std::string &response);
		void	setServer(Server* serv);
		void	setLocation(Location* loc);
		void	setPathFile(const std::string& str);
		void	setResponseHeaderStatus(const int& code);
		void	setResponseHeaderLocation(const std::string& str);
		bool	checkResponseHeaderConnectionClose();
		void	upFWrite();
		void	downFWrite();
//READ_SOCKET__________________________________________________________________
		void	readEraseHeaderRequest();
		bool	readSocketCheckEndRead();
		void	readExecChunk();
//WRITE_SOCKET_________________________________________________________________
		int		writeSocket();
//FIND_LOCATION________________________________________________________________
		int		locationFind();
		int		locationFindDirectory();
		void	locationFindExtension();
//RESPONSE_____________________________________________________________________
		int		check301();
		int		check413();
		bool	check501();
		bool	check405();
//GET__________________________________________________________________________
		int		runGet();
		int		getOpenFile(const std::string& str);
		void	getResponseHeaderContentType(const std::string& str);
//POST_________________________________________________________________________
		int		runPost();
//DELETE_______________________________________________________________________
		int		runDelete();
//AUTOINDEX____________________________________________________________________
		int		autoindexRenameFile(std::vector<std::string>& name_file);
		void	autoindexCreateBody(std::vector<std::string>& name_file);
//CGI__________________________________________________________________________
		int		runCgi();
		int		cgiFork(char** arg, char** env);
		int		cgiProcessingWriteRead(const int& pid,
					const int& fd_in, const int& fd_out);
		int		cgiWrite(int fd_in);
		int		cgiWriteAdd(const int& fd_in, const std::string& tmp);
		int		cgiRead(const int& fd_out, int& status_code, int& f_header);
		int		cgiFindHeader(int& status_code);
		void	cgiRetrunSaveStdFd(const int& in, const int& out);
		char**	cgiCreateArg();
		char**	cgiCreateEnv(void);
		void	cgiEnv();
		void	cgiFurunkul();
		int		cgiParserHeader(std::string str);
//ADD__________________________________________________________________________
		void	responseHeaderConnection(void);
		void	responseTotal(void);
		void	debugInfConnect(int socket_listen);
		void	debugInf(const std::string& mes) const;
		void	debugInfClose(const std::string& mes) const;
		bool	checkHeaderRequestChunk();
		void	findQueryStringPathInfo();
		bool	checkCgiTester();
		void	createJsonRequestHeader(const std::string& str);
		char**	freeEnv(char** env);
//DEBUG________________________________________________________________________
		bool	print_status_wait(const int& status);
		void	debug_show_map(std::map<std::string, std::string>& m);
		void	debug_show_arg(char** env);

	private:
		int									cSocket;
		time_t								timeStart;
		std::string							request;
		std::string							response;
		std::string							header;
		std::string							body;
		int									readByte;
		std::map<std::string, std::string>	jsonRequest;
		Server*								server;
		Location*							location;
		std::string							pathFile;
		std::map<std::string, std::string>	responseHeader;
		std::map<std::string, std::string>	envCgi;
		struct sockaddr_in					addr;
		bool								fWrite;
		int									chunked;
};

#endif
