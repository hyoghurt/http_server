#include <iostream>

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


str = "  abc	jd	l ";

std::vector<std::string>	func(const std::string& str)
{
	std::vector<std::string>		str;

}


int	main()
{
	std::cout << check_host("122.0.0.1:9000") << std::endl;
	std::cout << check_host("122.0.0.1:-1") << std::endl;
	std::cout << check_host("") << std::endl;
	std::cout << check_host("122.0.0.1:a") << std::endl;
	std::cout << check_host("256.0.0.1:3") << std::endl;
	std::cout << check_host("122.256.0.1:3") << std::endl;
	std::cout << check_host("122.2.256.1:3") << std::endl;
	std::cout << check_host("122.2.2.256:3") << std::endl;
	std::cout << check_host("122a.2.2.0:3") << std::endl;
	std::cout << check_host("122.2.2.0:") << std::endl;
	std::cout << check_host("122.2.2.0.23:34") << std::endl;
	std::cout << check_host("122.2.2.0.23") << std::endl;
	std::cout << check_host("122.0.23:34") << std::endl;
	std::cout << check_host("a.2.2.0.23") << std::endl;
	std::cout << check_host("122.2.2.:23") << std::endl;
}
