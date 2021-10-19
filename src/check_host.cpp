#include <iostream>
#include <vector>

int check_host(const std::string &host) {

   int positions_after_point[5] = {0};

   try {
      //Ищем все индексы точек и двоеточия и записываем их в массив интов
      for (int i = 0, j = 1; i != host.length(); i++) {
         if (j < 4 && host[i] == '.') {
            positions_after_point[j++] = i + 1;
         }
         else if (host[i] == ':') {
            if (j != 4) { return 2; } //more than 4 elem
            positions_after_point[j++] = i + 1;
         }
         else if (!isdigit(host[i])) { return 1; }
      }

      //Для каждого индекса делает сабстроку и переводим ее в инт
      for (int i = 0; i < 5; i++) {
         std::string b = host.substr(positions_after_point[i],
                              positions_after_point[i + 1] - positions_after_point[i] - 1);
         int res = stoi(b);
         if (i < 4 && (res < 0 || res > 255)) { return 3; } //elem go from range
//       std::cout << res << std::endl;
      }
   }
   catch (...) { return 4; }
   return 0;
}

std::vector<std::string>	split_by_space(const std::string& str)
{
   std::vector<std::string>		res;
   std::string const delims =  " \t";

   size_t begin, pos = 0;
   while ((begin = str.find_first_not_of(delims, pos)) != std::string::npos)
   {
      pos = str.find_first_of(delims, begin + 1);
      res.push_back(str.substr(begin, pos - begin));
   }
   return res;
}


int	main()
{
	std::vector<std::string>			v;
	std::vector<std::string>::iterator	it;
	
	v = split_by_space("ab ");

	for (it = v.begin(); it != v.end(); ++it)
		std::cout << "[" << *it << "]\n";
	std::cout << "----------------------\n";

	v = split_by_space("	 ");

	for (it = v.begin(); it != v.end(); ++it)
		std::cout << "[" << *it << "]\n";
	std::cout << "----------------------\n";

	v = split_by_space("a bc c	 ");

	for (it = v.begin(); it != v.end(); ++it)
		std::cout << "[" << *it << "]\n";
	std::cout << "----------------------\n";

	v = split_by_space("a");

	for (it = v.begin(); it != v.end(); ++it)
		std::cout << "[" << *it << "]\n";
	std::cout << "----------------------\n";

	v = split_by_space("");

	for (it = v.begin(); it != v.end(); ++it)
		std::cout << "[" << *it << "]\n";
	std::cout << "----------------------\n";

	v = split_by_space(" a b ");

	for (it = v.begin(); it != v.end(); ++it)
		std::cout << "[" << *it << "]\n";
	std::cout << "----------------------\n";

	v = split_by_space("a b ");

	for (it = v.begin(); it != v.end(); ++it)
		std::cout << "[" << *it << "]\n";
	std::cout << "----------------------\n";

	v = split_by_space(" a b");

	for (it = v.begin(); it != v.end(); ++it)
		std::cout << "[" << *it << "]\n";
	std::cout << "----------------------\n";
}
