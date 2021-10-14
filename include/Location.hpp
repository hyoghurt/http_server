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
			accessMethods.push_back("GET");
			return_code = 0;
			return_location = "";
			cgiPass = "";
			uploadPass = false;
			dowloadPass = false;
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
			this->uploadPass = oth.uploadPass;
			this->dowloadPass = oth.dowloadPass;
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
			accessMethods.push_back("GET");
			return_code = 0;
			return_location = "";
			cgiPass = "";
			uploadPass = false;
			dowloadPass = false;
			clientMaxBodySize = -1;
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
		bool						uploadPass;
		bool						dowloadPass;
		int							clientMaxBodySize;
};

#endif
