/*
1	Создать сокет
2	Привязать сокет к сетевому интерфейсу
3	Прослушивать сокет, привязанный к определенному сетевому интерфейсу
4	Принимать входящие соединения
5	Реагировать на события происходящие на сокетах
_______________________________________________________________________

https://www.opennet.ru/docs/RUS/linux_base/node244.html
socket - функция используется для создания сокета

____избежать залипание порта__
setsockopt - установить флаги на сокете

bind - функция используется сервером для присваивания сокету имени.
		До выполнения функции bind (т.е. присваивания какого-либо имени,
		вид которого зависит от адресного домена ) сокет недоступен
		программам-клиентам.

listen - Функция используется сервером, чтобы информировать ОС, что он ожидает
		("слушает") запросы связи на данном сокете. Без такой функции
		всякое требование связи с этим сокетом будет отвергнуто.

accept - функция используется сервером для принятия связи на сокет.
		Сокет должен быть уже слушающим в момент вызова функции.
		Если сервер устанавливает связь с клиентом, то функция возвращает
		новый сокет-дескриптор, через который и происходит общение
		клиента с сервером. Пока устанавливается связь клиента с сервером,
		функция accept блокирует другие запросы связи с данным сервером,
		а после установления связи "прослушивание" запросов возобновляется. 

recv -	Функция служит для чтения данных из сокета
send -	Функция служит для записи данных в сокет


getsockname
inet_addr
fcntl


select.	И опять его роль – это совместимость с иными платформами.
		Так же не быстр, так как срабатывает (возвращает управление)
		при событии на любом из сокетов, за которыми он наблюдает.
		После такого срабатывания нужно пробежать по всем и посмотреть
		на каком из сокетов произошло событие. Обобщая: одно
		срабатывание – это пробег по всему пулу наблюдаемых сокетов.

poll.	 более быстродейственный механизм, но не расчитан на большое
		количество сокетов для наблюдения.

epoll(Linux системы) и kqueue (FreeBSD). – примерно одинаковые механизмы,
		но яростные поклонники FreeBSD на некоторых форумах очень горячо
		твердят, что kqueue куда могучее. Не будем разжигать священные войны…
		Эти механизмы можно считать основными при написании высоконагруженных
		серверных приложений в *nix системах. Если описать кратко их принцип
		работы и он же достоинство – они возвращают некоторый объем
		информации, относящейся только к тем сокетам, на которых что-то
		произошло и не надо бегать по всем и проверять, что и где случилось.
		Так же эти механизмы расчитаны на большее количество одновременно
		обслуживаемых подключений.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
//#include <sys/un.h>
#include <sstream>
#include <fstream>

#define	GREEN "\033[1;38;5;2m"
#define NO_C "\033[0m"

void			print_error(const std::string& str);
std::string		ret_str_open(const std::string& file);

int	main()
{
	//создали сокет___________________________________________________
	//int socket(int domain, int type, int protocol)
	//IF_INET - IPv4 протоколы Интернет
	//SOCK_STREAM - обеспечивает создание двусторонних надежных и последовательных
	//				потоков байтов , поддерживающих соединения
	int	ls = socket(AF_INET, SOCK_STREAM, 0);
	if (ls == -1)
	    print_error("socket");

	//перевод сокета в неблокирующий режим____________________________
	//нужн перевести слушащий и клиентский сокет в данный режим_______
	int	flags = fcntl(ls, F_GETFL);
	fcntl(ls, F_SETFL, flags | O_NONBLOCK);

	//установили флаг, для предотвращения залипаниz TCP порта__________
	int	opt = 1;
	if (-1 == setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	    print_error("setsockopt");

	//сопостовление созданному сокету конкретного адреса________________
	//int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
	struct sockaddr_in  addr;

	addr.sin_family = AF_INET; //семейство адресов (IF_INET - IPv4 протоколы Интернет)
	addr.sin_port = htons(8000); //номер портa (8000)
	addr.sin_addr.s_addr = INADDR_ANY; //IP адрес (INADDR_ANY это 0.0.0.0 (все адреса))

	if (-1 == bind(ls, (struct sockaddr*) &addr, sizeof(addr)))
	{
		close(ls);
        print_error("bind");
	}

	//переводим сокет в слушаюший режим_________________________________
	if (-1 == listen(ls, 5))
	{
		close(ls);
        print_error("listen");
	}

	char			buf[1024];
	int				result;
	int				max_d;
	int				cl_socket;
	int				res;
    std::string		head;
	std::string		body;
    std::string		response;

	//множество дескрипторов_________________________________________
	fd_set			readfds;

	//хранилище клиентских сокетов____________________________________
	std::vector<int>			store_fd;
	std::vector<int>::iterator	it;

	//мультиплексирование ввода-вывода
	//select() poll() kqueue()
	while(1)
	{
		max_d = ls;

		FD_ZERO(&readfds); //очищаем множество
		FD_SET(ls, &readfds); //добавляем дескриптор в множество

		//пробегаем по хранилищу клиентских сокетов для добавления их в множество
		for(it = store_fd.begin(); it != store_fd.end(); ++it)
		{
			FD_SET(*it, &readfds); //добавляем дескриптор в множество
			if (*it > max_d)
				max_d = *it;
		}

		//создаем выборку событий при изменений состояния файлового дескриптора
		//(появление данных, доступных для чтения, запрос на соединение ...)
		//предыдущая выборка изменяется и в выборке
		//остаются лишь те дескрипторы которых пощекотали
		res = select(max_d + 1, &readfds, NULL, NULL, NULL);
		//if (res < 1)

		//если получили запрос от клиента__________________________________
		if (FD_ISSET(ls, &readfds)) //дескриптор слушающий сокет есть в выборке
		{
			cl_socket = accept(ls, NULL, NULL); //получили сокет клиента,
			//через который будет осуществлятся связь с клиентом
			if (cl_socket == -1)
				print_error("accept");
			store_fd.push_back(cl_socket); //сохранили сокет клиента
		}

		//пробегаем по хранилищу сокетов_____________________________________________
		for(it = store_fd.begin(); it != store_fd.end(); ++it)
		{
			if (FD_ISSET(*it, &readfds)) //если дескриптор входит в множество
			{
				cl_socket = *it;

				//читаем данные с клиентского сокета в buf____________________________
				result = recv(cl_socket, buf, sizeof(buf) - 1, 0);
				//if (result == -1)

				//debag_______________________________________________________________
				std::cout << GREEN << cl_socket << " result = " << result << NO_C << std::endl;
				//____________________________________________________________________

				//subject____________________
				//Checking the value of errno is strictly forbidden after a read or a write operation.
				//You server should have default error pages if none are provided.
				//Your program should have a config file in argument or use a default path.
				//You should be able to serve a fully static website.
				//Client should be able to upload files.
				//Your HTTP response status codes must be accurate.
				//You need at least GET, POST, and DELETE methods.
				//Your server can listen on multiple ports (See config file).
				//___________________________

				if (result > 0) //есть данные
				{
					buf[result] = '\0';
					std::cout << buf << std::endl;

					body = ret_str_open("www/index.nginx.html");

					//header ответа____________________________________________________
					head = "HTTP/1.1 200 OK\r\n";
    				head += "Content-Type: text/html; charset=UTF-8\r\n";
					head += "Connection: close\r\n";
					head += "Content-Length: ";
					head += std::to_string(body.length());
					head += "\r\n\r\n";

					//итоговый ответ___________________________________________________
					response = head + body;

					//отправляем ответ_________________________________________________
    				result = send(cl_socket, response.c_str(), response.length() + 1, 0);

					//shutdown(cl_socket, 2);
				}
				/*
				else if (result == 0)
				{
					store_fd.erase(it);
					close(cl_socket);
				}
				*/
			}
		}
	}
	std::cout << "end" << std::endl;
    close (ls);
	return (0);
}

void print_error(const std::string& str)
{
    std::cerr << "webserver: " << str << ": " << strerror(errno) << std::endl;
    exit (1);
}

std::string		ret_str_open(const std::string& file)
{
	/*
	std::stringstream my_str;
	my_str << "lkjijk";
	std::cout << my_str.str() << std::endl;
	*/
	std::string			str;
	std::string			res_str;
	std::ifstream		ifs(file);

	if (!ifs)
		//write correct
		return ("error\n");
	while (ifs)
	{
		getline(ifs, str);
		res_str += str;
		if (ifs)
			res_str += "\n";
	}
	ifs.close();
	return (res_str);
}
