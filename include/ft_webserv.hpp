#ifndef FT_WEBSERW_HPP
# define FT_WEBSERW_HPP

# define GREEN "\033[1;38;5;2m"
# define RED "\033[1;38;5;1m"
# define PINK "\033[1;48;5;5m"
# define NO_C "\033[0m"

# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <cstring>
# include <cerrno>
# include <iostream>
# include <vector>
# include <unistd.h>
# include <fcntl.h>
//#include <sys/un.h>
# include <sstream>
# include <fstream>
# include <ctime>
# include <vector>
# include <iomanip>
# include "Cl_socket.hpp"

void			print_error(const std::string& str);
int				create_listen_socket(void);
std::string		ret_str_open(const std::string& file);
void			debag_pring_request(const int& fd_client, const std::string& str);
std::string		get_new_time(void);
void			print_connect_info(int fd_cl, struct sockaddr_in addr_cl);

#endif
