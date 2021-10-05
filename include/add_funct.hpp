#ifndef ADD_FUNCT_HPP
# define ADD_FUNCT_HPP

# include <iostream>
# include <cerrno>
//#include <ctime>
# include <sys/time.h>
# include <arpa/inet.h> //sockaddr_in
# include <cstring> //strerror

# include "def_color.hpp"

void			print_debug(const std::string& str);
void			print_info(const std::string& str);
void			print_error(const std::string& str);
void			print_error_strerror(const std::string& str);
void			print_connect_info(int socket_listen, int fd_cl, struct sockaddr_in addr_cl);
std::string		get_new_time(void);

#endif
