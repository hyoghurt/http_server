#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <vector>

class	Location
{
	public:
		Location ()
		{
			root = "";
			autoindex = false;
			index = "index.html";
			accessMethods.push_back("GET");
			return_code = 0;
			return_location = "";
			cgiPass = "";
			uploadPass = false;
			dowloadPass = false;
		}
		~Location () {}
		Location (const std::string& root, const bool autoindex,
				const std::string& index,
				const std::vector<std::string>& accessMethods)
		{
			this->root = root;
			this->autoindex = autoindex;
			this->index = index;
			this->accessMethods = accessMethods;
		}

	public:
		std::string					root;
		bool						autoindex;
		std::string					index;
		std::vector<std::string>	accessMethods;
		int							return_code;
		std::string					return_location;
		std::string					cgiPass;
		bool						uploadPass;
		bool						dowloadPass;
};

#endif
