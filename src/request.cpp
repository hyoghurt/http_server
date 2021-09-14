# include <vector>
# include <iostream>

std::vector<std::string>	split_request(const std::string& str)
{
	std::vector<std::string>	result;
	std::string					s;
	std::string					find(" \n");
	size_t						pos;
	size_t						pos_n;
	size_t						start(0);
	size_t						start_n(0);
	std::string					tmp;

	/*
	pos_n = str.find("\n", start_n);

	while (pos_n != std::string::npos)
	{
		tmp = str.substr(start_n, pos_n - start_n - 1);
		std::cout << "tmp = " << tmp << std::endl;

		start = 0;
		pos = tmp.find(" ", start);
		while (pos != std::string::npos)
		{
			std::cout << "[" << tmp.substr(start, pos - start) << "]" << std::endl;
			start = pos + 1;
			pos = tmp.find(" ", start);
		}
		std::cout << "[" << tmp.substr(start, pos - start) << "]" << std::endl;

		start_n = pos_n + 1;
		pos_n = str.find("\n", start_n);
	}
	*/
	pos = str.find(" ", start);
	result.push_back(str.substr(start, pos - start));

	start = pos + 1;
	pos = str.find(" ", start);
	result.push_back(str.substr(start, pos - start));

	pos = str.find("\nHost: ", start);
	start = pos + 7;
	pos = str.find("\r", start);
	result.push_back(str.substr(start, pos - start));

	std::vector<std::string>::iterator	it;

	for (it = result.begin(); it != result.end(); ++it)
		std::cout << "[" << *it << "]" << std::endl;

	return (result);
}
