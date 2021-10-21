#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <vector>

class	Location
{
	public:
		Location ()
		{
			rule = "";
			root = "";
			autoindex = false;
			index = "index.html";
			return_code = 0;
			return_location = "";
			cgiPass = "";
			clientMaxBodySize = -1;
		}
		Location (const Location& oth)	{ *this = oth; }

		~Location ()					{}

		Location&	operator= (const Location& oth)
		{
			this->rule = oth.rule;
			this->root = oth.root;
			this->autoindex = oth.autoindex;
			this->index = oth.index;
			this->accessMethods = oth.accessMethods;
			this->return_code = oth.return_code;
			this->return_location = oth.return_location;
			this->cgiPass = oth.cgiPass;
			this->clientMaxBodySize = oth.clientMaxBodySize;
			return *this;
		}

		void	clear()
		{
			rule = "";
			root = "";
			autoindex = false;
			index = "index.html";
			accessMethods.clear();
			return_code = 0;
			return_location = "";
			cgiPass = "";
			clientMaxBodySize = -1;
		}

		const std::string&	getRule() const
		{ return this->rule; }

		const std::string&	getRoot() const
		{ return this->root; }

		const std::string&	getIndex() const
		{ return this->index; }

		const int&	getReturnCode() const
		{ return this->return_code; }

		const std::string&	getReturnLocation() const
		{ return this->return_location; }

		const int&	getClientMaxBodySize() const
		{ return this->clientMaxBodySize; }

		const std::string&	getCgiPass() const
		{ return this->cgiPass; }

		bool		checkAccessMethod(const std::string& method)
		{
			std::vector<std::string>::iterator	it;

			for (it = accessMethods.begin(); it != accessMethods.end(); ++it)
				if (*it == method)
					return (true);
			return (false);
		}

	public:
		std::string					rule;
		std::string					root;
		bool						autoindex;
		std::string					index;
		std::vector<std::string>	accessMethods;
		int							return_code;
		std::string					return_location;
		std::string					cgiPass;
		int							clientMaxBodySize;
};

#endif
