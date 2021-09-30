#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <vector>

class	Location
{
	public:
		Location () {}
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
};

#endif
