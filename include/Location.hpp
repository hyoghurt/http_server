#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <vector>

class	Location
{
	public:
		Location ();
		Location (const Location& oth);
		~Location ();

		Location&			operator= (const Location& oth);
		void				clear();
		bool				checkAccessMethod(const std::string& method);

		const std::string&	getRule() const;
		const std::string&	getRoot() const;
		const bool&			getAutoindex() const;
		const std::string&	getIndex() const;
		const int&			getReturnCode() const;
		const std::string&	getReturnLocation() const;
		const std::string&	getCgiPass() const;
		const int&			getClientMaxBodySize() const;
		void				setRule(const std::string& str);
		void				setRoot(const std::string& str);
		void				setAutoindex(const bool& b);
		void				setIndex(const std::string& str);
		void				setAccessMethods(const std::vector<std::string>& v);
		void				setReturnCode(const int& i);
		void				setReturnLocation(const std::string& str);
		void				setCgiPass(const std::string& str);
		void				setClientMaxBodySize(const int& i);

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
