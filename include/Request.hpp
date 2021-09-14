#ifndef REQUEST_HPP
# define REQUEST_HPP

std::vector<std::string>	split_request(const std::string& str);

class	Request
{
	public:
		std::string	method;
		std::string	path;
		std::string	host;
		std::string	body;

		Request() {}

		Request (const std::string& str)
		{
			std::vector<std::string>	tmp;

			tmp = split_request(str);
			method = tmp[0];
			path = tmp[1];
		}

		Request (const Request& oth)
		{ *this = oth; }

		~Request() {}

		Request&	operator= (const Request& oth)
		{
			this->method = oth.method;
			this->path = oth.path;
			this->host = oth.host;
			this->body = oth.body;
			return *this;
		}
};

#endif
