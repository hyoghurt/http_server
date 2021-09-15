#ifndef CL_SOCKET_HPP
# define CL_SOCKET_HPP

class	Cl_socket
{
	public:
		int			fd;
		time_t		time_start;
		std::string	request;
		std::string	response;

		Cl_socket() : fd(0)
		{
			time(&time_start);
		}

		Cl_socket(int fd) : fd(fd)
		{
			time(&time_start);
			request = "";
		}

		Cl_socket(const Cl_socket& oth)
		{ *this = oth; }

		~Cl_socket() { }

		Cl_socket&	operator= (const Cl_socket& oth)
		{
			this->fd = oth.fd;
			this->time_start = oth.time_start;
			return *this;
		}
};

#endif
