#ifndef RESPONSE_HPP
# define RESPONSE_HPP

class	Request;

class	Response
{
	public:
		std::string	status_code;
		std::string	content_type;
		std::string	connection;
		std::string	content_length;

		std::string	body;

		Response () {}
		Response (const Request& oth)
		{
			//header ответа____________________________________________________
			//body = ret_str_open("www/index.nginx.html");
			body = "sdfd";

			status_code = "200 OK";
    		content_type= "text/html; charset=UTF-8";
			connection = "close";
			content_length = std::to_string(body.length());
		}
		Response (const Response& oth) { *this = oth; }
		~Response () {}

		Response&	operator= (const Response& oth) 
		{
			if (this != &oth)
			{
				this->status_code = oth.status_code;
				this->content_type = oth.content_type;
				this->connection = oth.connection;
				this->content_length = oth.content_length;
				this->body = oth.body;
			}
			return *this;
		}

		const std::string	get_response()
		{
			std::string		result;

			result = "HTTP/1.1 ";
			result += status_code;
			result += "\r\n";

    		result += "Content-Type: ";
			result += content_type;
			result += "\r\n";

			result += "Connection: ";
			result += connection;
			result += "\r\n";

			result += "Content-Length: ";
			result += content_length;
			result += "\r\n\r\n";

			result += body;

			return (result);
		}
};

#endif
