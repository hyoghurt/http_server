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
#include <ctime>
#include <vector>
#include "ft_webserv.hpp"

class	cl_socket
{
	private:
		int		fd;
		time_t	time_start;

	public:
		cl_socket() : fd(0)
		{ time(&time_start); }

		cl_socket(int fd) : fd(fd)
		{ time(&time_start); }

		cl_socket(const cl_socket& oth)
		{ *this = oth; }

		~cl_socket() { }

		cl_socket&	operator= (const cl_socket& oth)
		{
			this->fd = oth.fd;
			this->time_start = oth.time_start;
			return *this;
		}
		void	set_fd(int fd)
		{ this->fd = fd; }

		int		get_fd(void) const
		{ return this->fd; }

		void	set_time(void)
		{ time(&time_start); }

		time_t	get_time(void) const
		{ return this->time_start; }
};

int				create_listen_socket(void);
std::string		ret_str_open(const std::string& file);

int	main()
{
	//хранилище клиентских сокетов____________________________________
	std::vector<cl_socket>				store;
	std::vector<cl_socket>::iterator	it_store;
	int									ls;

	ls = create_listen_socket();

	char			buf[1024];
	int				result;
	int				max_d;
	int				fd_client;
	int				res;
    std::string		head;
	std::string		body;
    std::string		response;

	//множество дескрипторов_________________________________________
	fd_set			readfds;

	//мультиплексирование ввода-вывода
	//select() poll() kqueue()
	while(1)
	{
		max_d = ls;

		FD_ZERO(&readfds); //очищаем множество
		FD_SET(ls, &readfds); //добавляем дескриптор в множество

		//пробегаем по хранилищу клиентских сокетов для добавления их в множество
		for(it_store = store.begin(); it_store != store.end(); ++it_store)
		{
			fd_client = cl_socket(*it_store).get_fd();

			FD_SET(fd_client, &readfds); //добавляем дескриптор в множество
			if (fd_client > max_d)
				max_d = fd_client;
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
			fd_client = accept(ls, NULL, NULL); //получили сокет клиента,
			//через который будет осуществлятся связь с клиентом
			if (fd_client == -1)
				print_error("accept");
			store.push_back(cl_socket(fd_client)); //save socket client
		}

		//пробегаем по хранилищу сокетов_____________________________________________
		for(it_store = store.begin(); it_store != store.end(); ++it_store)
		{
			fd_client = cl_socket(*it_store).get_fd();

			if (FD_ISSET(fd_client, &readfds)) //если дескриптор входит в множество
			{
				//save time
				cl_socket(*it_store).set_time();

				//читаем данные с клиентского сокета в buf____________________________
				result = recv(fd_client, buf, sizeof(buf) - 1, 0);
				//if (result == -1)

				//debag_______________________________________________________________
				std::cout << GREEN << fd_client << " result = " << result << NO_C << std::endl;
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
    				result = send(fd_client, response.c_str(), response.length() + 1, 0);

					//shutdown(cl_socket, 2);
				}
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
