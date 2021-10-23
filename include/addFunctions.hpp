#ifndef ADDFUNCTIONS_HPP
# define ADDFUNCTIONS_HPP

# include <iostream>
# include <cerrno>
# include <sys/time.h>
# include <arpa/inet.h> //sockaddr_in
# include <cstring> //strerror
# include <sys/stat.h> //stat or <sys/types.h> <unistd.h>
# include <sstream> //std::istringstream
# include <vector>

# include "def_color.hpp"

int							print_error(const std::string& str);
int							print_error_strerror(const std::string& str);
std::string					get_new_time();
int							check_dir_or_file(const std::string& name_file);
std::string					convert_base16_to_str(const size_t& n);
size_t						convert_str_to_base16(const std::string& str);
std::vector<std::string>	split_by_space(const std::string& str);
std::string					absolutePathOfExec(const std::string& execName);
int							check_host(const std::string &host);
std::string					get_status_code(const int& code);
std::pair<std::string, std::string>		returnKeyVal(const std::string& str);

#endif
