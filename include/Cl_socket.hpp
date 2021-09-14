#ifndef CL_SOCKET_HPP
# define CL_SOCKET_HPP

# include "Request.hpp"
# include "Response.hpp"

class	Cl_socket
{
	private:
		int			fd;
		time_t		time_start;
		Request		request;
		Response	response;

	public:
		Cl_socket() : fd(0)
		{ time(&time_start); }

		Cl_socket(int fd) : fd(fd)
		{ time(&time_start); }

		Cl_socket(const Cl_socket& oth)
		{ *this = oth; }

		~Cl_socket() { }

		Cl_socket&	operator= (const Cl_socket& oth)
		{
			this->fd = oth.fd;
			this->time_start = oth.time_start;
			return *this;
		}
		void	set_fd(int fd)
		{ this->fd = fd; }

		int		get_fd(void) const
		{ return this->fd; }

		void	set_time(void)
		{ time(&time_start); }

		time_t	get_time(void) const
		{ return this->time_start; }

		Request	set_request(const Request& oth)
		{ return this->request = oth; }
};

#endif
