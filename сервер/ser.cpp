
/**
*  
*  Пример TCP клиента и сервера.
*
*  Файлы
*  cli_tcp.cpp  - пример TCP клиента
*  ser_tcp_select.cpp - пример TCP сервера c использованием select()
*
*  Клиент в цикле получает с клавиатуры текстовую строку и отсылает
*  ее на сервер, читает и выводит на экран ответ сервера.
*  Сервер ждет соединений от клиентов. При установленом
*  соединении получает строку от клиента, переводит ее в верхний
*  регистр и отсылает обратно клиенту. Если клиент посылает строку
*  содержащую слово stop, то сервер закрывает соединение с этим клиентом,
*  а клиент заканчивает свою работу.
*  Клиент и сервер печатают на экран протокол своей работы т.е.
*  разные данные, которые они получают или отсылают.
*
*
*/


//#include "stdafx.h"
#pragma warning(suppress : 4996)
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS



#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <fstream>
#include <sstream>
#include<vector>
#include<iostream>
#include <io.h>
//#include <fcntl.h>
#include <stdint.h>
#include<cstdlib>

#include"Header.h"
//using namespace std;



// Определимся с номером порта и другими константами.
#define PORT    5555
//#define PORT    5556
#define BUFLEN  4096

// Две вспомогательные функции для чтения/записи (см. ниже)
int   readFromClient(int fd, char* buf);
void  writeToClient(int fd, char* buf, Avia* pA, int n);

int   readFromClientHTTP(int fd, char* buf);
void  writeToClientHTTP(int fd, char* buf, Avia* pA, int n);

void Generate(FILE* f, int quantity, string* city, string* aviacompany, int size_city, int size_aviacompany, Avia* A);
void Search(vector<int>& str_res, Avia* A, string x);
void Add(Avia* A, string s, int size);
void Delete(Avia* A, string s, int size);
int Buy(Avia* A, string s, int size);

int  main(void)
{
	SetConsoleCP(1251);
	setlocale(LC_CTYPE, "rus");
	setlocale(LC_ALL, "Russian");
    FILE* out; // файл для записи базы
    out = fopen("out.txt", "w");
    if (!out)
    {
        cout << "Error open file out.txt" << endl;
        return -1;
    }

    ifstream f;
    f.open("city.txt");
    string city[327];
    for (int i = 0; i < 327; ++i)
    {
        f >> city[i];
    }


    ifstream f4;
    f4.open("company.txt");
    string company[20];
    for (int i = 0; i < 20; ++i)
    {
        f4 >> company[i];
    }

  //  int n = 100100;
	int n = 10100;
    Avia* pA = new Avia[n];
    Generate(out, n, city, company, 327, 20, pA);
	fclose(out);


    int     i, err, opt = 1;
    int     sock, new_sock;
    fd_set  active_set, read_set;
    struct  sockaddr_in  addr;
    struct  sockaddr_in  client;
    char    buf[BUFLEN];
    socklen_t  size;
	//vector<int> clients;
    printf("http SERVER\n");

    // Инициализация windows sockets
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Создаем TCP сокет для приема запросов на соединение
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Server: cannot create socket");
        exit(EXIT_FAILURE);
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // Заполняем адресную структуру и
    // связываем сокет с любым адресом
   addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
	//addr.sin_port = htons(Port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    err = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (err < 0) {
        perror("Server: cannot bind socket");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Создаем очередь на 3 входящих запроса соединения
    err = listen(sock, 3);
    if (err < 0) {
        perror("Server: listen queue failure");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

	//struct timeval tv; /* структура для задания таймаута */
    // Подготавливаем множества дескрипторов каналов ввода-вывода.
    // Для простоты не вычисляем максимальное значение дескриптора,
    // а далее будем проверять все дескрипторы вплоть до максимально
    // возможного значения FD_SETSIZE.
    FD_ZERO(&active_set);
    FD_SET(sock, &active_set);

    // Основной бесконечный цикл проверки состояния сокетов
    while (1) {
		SetConsoleCP(1251);
		setlocale(LC_CTYPE, "rus");
		setlocale(LC_ALL, "Russian");
        // Проверим, не появились ли данные в каком-либо сокете.
        // В нашем варианте ждем до фактического появления данных.
        read_set = active_set;
        if (select(FD_SETSIZE, &read_set, NULL, NULL, NULL) < 0) {
            perror("Server: select  failure");
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        // fd_set в windows устроена немного иначе - как массив дескрипторов
        // Данные появились. Проверим в каком сокете.
        for (int j = 0; j < read_set.fd_count; j++) {
            i = read_set.fd_array[j];
            if (FD_ISSET(i, &read_set)) {
                if (i == sock) {
                    // пришел запрос на новое соединение
                    size = sizeof(client);
                    new_sock = accept(sock, (struct sockaddr*)&client, &size);
                    if (new_sock < 0) {
                        perror("accept");
                        WSACleanup();
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stdout, "Server: connect from host %s, port %hu.\n",
                        inet_ntoa(client.sin_addr),
                        ntohs(client.sin_port));
                    FD_SET(new_sock, &active_set);
                    fprintf(stdout, "new socket = %d\n", new_sock);
                }
                else {
                    fprintf(stdout, "socket = %d\n", i);
                    // пришли данные в уже существующем соединени
                	err = readFromClientHTTP(i, buf);
					if (err < 0) {
                        // ошибка или конец данных
                        closesocket(i);
                        FD_CLR(i, &active_set);
						std::fprintf(stdout, "close socket = %d\n", i);
                    }
                    else {
                        // а если это команда закончить работу?
                        if (strstr(buf, "stop")) {
                            closesocket(i);
                            FD_CLR(i, &active_set);
							std::fprintf(stdout, "close socket = %d\n", i);
                        }
                        else {
                            // данные прочитаны нормально
							writeToClientHTTP(i, buf, pA, n);
                        }
                    }
                }
            }
        }
    }
    WSACleanup();
 
}

int  readFromClientHTTP(int fd, char* buf)
{
	int  nbytes;

	nbytes = recv(fd, buf, BUFLEN, 0);
	if (nbytes > 0) { fprintf(stdout, "reading %d bytes from socket %d\n", nbytes, fd); }
	else {
		// ошибка чтения
		int err = WSAGetLastError();
		printf("Socket read error %d\n", err);
		return -1;
	}
	if(nbytes > 0){
		// есть данные
		printf("\n\nServer gets %d bytes:\n", nbytes);
		// поставим 0 в конце прочитанных данных
		buf[min(nbytes, BUFLEN - 1)] = 0;
		printf("%s\n", buf);
		return nbytes;
	}
}

#include <time.h>

void ProcessHTML(stringstream& html, string& request)
{
	// пример - распечатка времени в поле comment  
#define TBUFLEN 64    // никогда не забывайте определять константы для длин строк              
					  // ошибка на лекции была как раз из-за того, что при определении
					  // массива tbuf и при его использовании в вызове ctime_s 
					  // по недосмотру и небрежности оказались разные явные значения (64 и 256)
	char tbuf[TBUFLEN] = { 0 };
	time_t ttt;
	time(&ttt);
	ctime_s(tbuf, TBUFLEN, &ttt);
	printf("%s\n", tbuf);

	html.clear();
	html << "<!DOCTYPE html>\r\n";
	html << "<html><head><meta charset = \"cp1251\">\r\n";
	html << "<title>Поиск и покупка авиабилетов</title></head>\r\n";
	html << "<body>\r\n";
	html << "<h2>Авиабилеты</h2>\r\n";
	//html << "<form name=\"myform\" action=\"http://localhost:5555\"  method=\"post\">\r\n";
	html << "<form name=\"myform\" action=\"http://localhost:5555\"  method=\"get\">\r\n";
	html << "<input name=\"request\" type=\"text\" width=\"20\"> request<br>\r\n";
	//html << "<input name=\"comment\" type=\"text\" width=\"20\" value=\"some text\"> comment<br>\r\n";
	html << "<input name=\"comment\" type=\"text\" width=\"20\" value=\"" << tbuf << "\"> comment<br>\r\n";
	html << "1 <input name=\"rb1\" type=\"radio\" value=\"check1\">\r\n";
	html << "2 <input name=\"rb1\" type=\"radio\" value=\"check2\" checked>\r\n";
	html << "3 <input name=\"rb2\" type=\"radio\" value=\"check3\">\r\n";
	html << "4 <input name=\"rb2\" type=\"radio\" value=\"check4\"><br>\r\n";
	html << "A <input name=\"ch1\" type=\"checkbox\" value=\"checkA\">\r\n";
	html << "B <input name=\"ch2\" type=\"checkbox\" checked>\r\n";
	html << "C <input name=\"ch3\" type=\"checkbox\"><br>\r\n";
	html << "<input name=\"sub\" type=\"submit\" value=\"Send\">\r\n";
	html << "</form>\r\n";
	html << "</body></html>\r\n";




}

void ProcessHTML1(stringstream& html, string& request, string x)
{
	// пример - распечатка времени в поле comment  
#define TBUFLEN 64    
	char tbuf[TBUFLEN] = { 0 };
	time_t ttt;
	time(&ttt);
	ctime_s(tbuf, TBUFLEN, &ttt);
	printf("%s\n", tbuf);

	html.clear();
	html << "<!DOCTYPE html>\r\n";
	html << "<html><head><meta charset = \"cp1251\">\r\n";
	html << "<title>Поиск и покупка авиабилетов</title></head>\r\n";
	html << "<body>\r\n";
	html << "<background-image :url('picture.jpg')\r\n";
	html << "<h2>Авиабилеты</h2>\r\n";
	//html << "<form name=\"myform\" action=\"http://localhost:5555\"  method=\"post\">\r\n";
	html << "<form name=\"myform\" action=\"http://localhost:5555\"  method=\"get\">\r\n";
	//html << "<input name=\"request\" type=\"text\" width=\"20\"> request<br>\r\n";
	html << "<input name=\"request\" type=\"text\" width=\"20\" placeholder = \"Введите запрос\" value = \" "<< x << " \"><font size=\"3\" color=\"black\" face=\"Arial\"> request</font><br>\r\n";
	//html << "<input name=\"comment\" type=\"text\" width=\"20\" value=\"some text\"> comment<br>\r\n";
	html << "<input name=\"comment\" type=\"text\" width=\"20\" value=\"" << tbuf << "\"> comment<br>\r\n";
	
	html << "<input name=\"sub\" type=\"submit\" value=\"Send\">\r\n";
	html << "</form>\r\n";
	html << "</body></html>\r\n";
}

void  writeToClientHTTP(int fd, char* buf, Avia* pA, int n)
{
	int  nbytes;
	stringstream head;
	stringstream html;
	string request = "";

	if (strstr(buf, "GET")) {
		char* p1 = strchr(buf, '/');
		char* p2 = strchr(p1, ' ');
		*p2 = 0;
		request = p1;
		printf("get request: %s\n", request.c_str());
		*p2 = ' ';
	}
	if (strstr(buf, "POST")) {
		char* p1 = strstr(buf, "\r\n\r\n");
		request = (p1 + 4);
		printf("post request: %s\n", request.c_str());
	}

	// ответ на нераспознанные запросы    
	if (request != "/" && request.find("request") == string::npos) {
		head << "HTTP/1.1 404 Not found\r\n";
		head << "Connection: close\r\n";
		head << "Content-length: 0\r\n";
		head << "\r\n";
		nbytes = send(fd, head.str().c_str(), head.str().length() + 1, 0);
		return;
	}

	std::cout << request << endl;
	int pos1 = request.find_first_of('=');
	int pos2 = request.find_first_of('&');
	string x = request.substr(pos1+1, pos2 - pos1-1);

	while (x.find_first_of('+')!= std::string::npos)
	{
		auto iter = x.find_first_of('+');
		x[iter] = ' ';
	}

	stringstream tmp;
	string command, number, city1, city2, date1, date2, site1, site2, cost, company;
	tmp << x;
	tmp >> command;
	tmp >> number;
	tmp >> city1;
	tmp >> city2;
	tmp >> date1;
	tmp >> date2;
	tmp >> site1;
	tmp >> site2;
	tmp >> cost;
	tmp >> company;
	string xx;
	xx = number + " " + city1 + " " + city2 + " " + date1 + " " + date2 + " " + site1 + " " + site2 + " " + cost + " " + company;
	std::cout << xx << endl;

	if (command == "Search")
	{
		vector<int> res;
		Search(res, pA, xx);
		int j = 0;
		html << "	<tr> Results of search </tr>\r\n";
		html << "<table>\r\n";
		html << "<tbody>\r\n";
		html << "<tr>\r\n";
		html << "<th>Number\r\n";
		html << "<th>Departure\r\n";
		html << "<th>Arrival\r\n";
		html << "<th>Date departure\r\n";
		html << "<th>Date arrrival\r\n";
		html << "<th>Free site\r\n";
		html << "<th>Occupied site\r\n";
		html << "<th>Price\r\n";
		html << "<th>Status\r\n";
		for (int index : res)
		{
			Avia str_str = pA[index];
			html << "<tr>\r\n";
			html << "<td>"<<str_str.number<<"\r\n";
			html << "<td>" << str_str.str_city_ar << "\r\n";
			html << "<td>" << str_str.str_city_dep << "\r\n";
			html << "<td>" << str_str.str_date_ar << "\r\n";
			html << "<td>" << str_str.str_date_dep << "\r\n";
			html << "<td>" << str_str.str_free_site << "\r\n";
			html << "<td>" << str_str.str_occupied_site << "\r\n";
			html << "<td>" << str_str.str_price << "\r\n";
			html << "<td>" << str_str.status << "\r\n";
			html << "<tr>\r\n";
		}
		html << "</tbody>\r\n";
		html << "</table>\r\n";
		cout << endl;
	}

	if (command == "Add") {
		string index;
		tmp >> index;
		xx = xx + " " + index;
		Add(pA, xx, n);
		html << "<table>\r\n";
		html << "<tbody>\r\n";
		html << "<tr>\r\n";
		html << "<th>Рейс добавлен\r\n";
		html << "</tbody>\r\n";
		html << "</table>\r\n";		
	}
	if (command == "Delete")
	{
		Delete(pA, xx, n);
		x = "";
		html << "<table>\r\n";
		html << "<tbody>\r\n";
		html << "<tr>\r\n";
		html << "<th>Рейс удален\r\n";
		html << "</tbody>\r\n";
		html << "</table>\r\n";
	}
	if (command == "Buy")
	{
		x.erase(x.begin(), x.begin() + 4);
		int i = Buy(pA, x, n);
		x = "";
		if (i == 1)
		{
			html << "<table>\r\n";
			html << "<tbody>\r\n";
			html << "<tr>\r\n";
			html << "<th>Билет куплен. Хорошего полета!\r\n";
			html << "</tbody>\r\n";
			html << "</table>\r\n";
		}
		else {
			html << "<table>\r\n";
			html << "<tbody>\r\n";
			html << "<tr>\r\n";
			html << "<th>К сожалению, все билеты на этот рейс раскуплены.\r\n";
			html << "</tbody>\r\n";
			html << "</table>\r\n";
		}
	}

	// ответ на распознанные запросы
	// формирование HTML
	//ProcessHTML(html, request);
	
		ProcessHTML1(html, request, x);
		int html_length = html.str().length();

		// формирование HTTP заголовка
		head << "HTTP/1.1 200 OK\r\n";
		head << "Connection: keep-alive\r\n";
		head << "Content-type: text/html\r\n";
		head << "Content-length: " << html_length << "\r\n";
		head << "\r\n";
 
		// вывод заголовка и html
		nbytes = send(fd, head.str().c_str(), head.str().length() + 1, 0);
		printf("http nb = %d\n", nbytes);
		nbytes = send(fd, html.str().c_str(), html.str().length() + 1, 0);
		printf("html nb = %d\n", nbytes);
	 
	//cout << "Write back\n";
	//cout << head.str();
	//cout << html.str();

	if (nbytes < 0) {
		perror("Server: write failure");
	}
}


int  readFromClient(int fd, char* buf)
{
	SetConsoleCP(1251);
	setlocale(LC_CTYPE, "rus");
	setlocale(LC_ALL, "Russian");
	int k;
    int  nbytes;

    nbytes = recv(fd, buf, BUFLEN, 0);
	std::fprintf(stdout, "reading %d bytes from socket %d\n", nbytes, fd);



    if (nbytes < 0) {
        // ошибка чтения
        perror("Server: read failure");
        return -1;
    }
    else if (nbytes == 0) {
        // больше нет данных
        return -1;
    }
    else {
        // есть данные
        std::fprintf(stdout, "Server got message: %s\n", buf);
        return 0;
    }
 
}


void  writeToClient(int fd, char* buf, Avia* pA, int n)
{
	SetConsoleCP(1251);
	setlocale(LC_CTYPE, "rus");
	setlocale(LC_ALL, "Russian");
    int  nbytes;
    unsigned char* s;

	std::string x = "";
	for (int i = 0; i < BUFLEN; ++i)
	{
		
		x += buf[i];
	}

	stringstream tmp;
	string command, number, city1, city2, date1, date2, site1, site2, cost, company;
	tmp << x;
	tmp >> command;
	tmp >> number;
	tmp >> city1;
	tmp >> city2;
	tmp >> date1;
	tmp >> date2;
	tmp >> site1;
	tmp >> site2;
	tmp >> cost;
	tmp >> company;
	string xx;
	xx = number + " " +city1 +" "+ city2+" " + date1+" " + date2+" "+  site1+" " + site2+" " + cost+" " + company;
	if (command == "Search")
	{
		
		
		vector<int> res;
		

		//x.erase(x.begin(), x.begin() + 7);
		Search(res, pA, xx);
		int j = 0;
		for (auto i : res)
		{
	 
			
			
			cout << pA[i];
			//buf[j] = pA[i];
			for (auto ch : pA[i].number)
			{
				buf[j] = ch;
				++j;
			}
			buf[j] = '.';
			++j;
			buf[j] = ' ';
			++j;
			for (auto ch : pA[i].str_city_ar)
			{
				buf[j] = ch;
				++j;
			}
			buf[j] = ' ';
			++j;

			for (int i = 0; i < pA[i].str_city_dep.size(); ++i)
			{
				if (company[i] == NULL)
				{
					pA[i].str_city_dep.erase(i, pA[i].str_city_dep.size());
					i = pA[i].str_city_dep.size();
				}
			}

			for (auto ch : pA[i].str_city_dep)
			{
				buf[j] = ch;
				++j;
			}
			buf[j] = ' ';
			++j;
			for (auto ch :pA[i].str_date_ar)
			{
				buf[j] = ch;
				++j;
			}
			
			buf[j] = ' ';
			++j;
			for (auto ch : pA[i].str_date_dep)
			{
				buf[j] = ch;
				++j;
			}
			buf[j] = ' ';
			++j;
			for (auto ch : to_string(pA[i].str_free_site))
			{
				buf[j] = ch;
				++j;
			}
			buf[j] = ' ';
			++j;
			for (auto ch : to_string(pA[i].str_occupied_site))
			{
				buf[j] = ch;
				++j;
			}
			buf[j] = ' ';
			++j;
			for (auto ch : to_string(pA[i].str_price))
			{
				buf[j] = ch;
				++j;
			}
			
			buf[j] = ' ';
			++j;
			for (auto ch : pA[i].str_company)
			{
				buf[j] = ch;
				++j;
			}
			buf[j] = ' ';
			++j;

			for (int i = 0; i < pA[i].status.size(); ++i)
			{
				if (company[i] == NULL)
				{
					pA[i].status.erase(i, pA[i].status.size());
					i = pA[i].status.size();
				}
			}

			for (auto ch : pA[i].status)
			{
				buf[j] = ch;
				++j;
			}
			buf[j] = '\n';
			++j;
			buf[j] = NULL;			

		}
		cout << endl;
		cout <<"................." <<buf <<"................."<< endl;
		cout << endl;
		//x = "";

	}
	
	if (command == "Add") {
		//x.erase(x.begin(), x.begin() + 4);
		string index;
		tmp >> index;
		xx = xx + " " + index;
		Add(pA, xx, n);
		//x = "";

	}
	if (command == "Delete")
	{
		//x.erase(x.begin(), x.begin() + 7);
		Delete(pA, xx, n);
		x = "";

	}
	

        int lenght=0;
		for (int i = 0; i < BUFLEN; ++i)
		{
			if (buf[i] == NULL)
			{
				lenght = i;
				i = BUFLEN;
			}
		}
		for (s = (unsigned char*)buf; *s; s++) *s = toupper(*s);
    nbytes = send(fd, buf, strlen(buf) + 1, 0);
//	nbytes = send(fd, buf, lenght, 0);
	std::fprintf(stdout, "Write back: %s\nnbytes=%d\n", buf, nbytes);

    if (nbytes < 0) {
        perror("Server: write failure");
    }

	

}

void Generate(FILE* f, int quantity, string* city, string* aviacompany, int size_city, int size_aviacompany, Avia* A)
{
	string departure, arrival, date_d, date_a, company;
	int free_site, occupied_site;
	int price;
	string status = "Active";
	for (int i = 0; i < quantity; i++)
	{
		departure = city[rand() % size_city];
		arrival = city[rand() % size_city];
		while (departure == arrival)
		{
			departure = city[rand() % size_city];
			arrival = city[rand() % size_city];
		}

		string year1, year2, month1, month2, day1, day2, hour1, hour2, minute1, minute2;
		year1 = "2022";
		year2 = "2022";
		month1 = to_string(6 + rand() % 3); //6 7 8 
		day1 = to_string(1 + rand() % 31);
		hour1 = to_string(rand() % 24);
		if (hour1.size() == 1)
		{
			hour1 = "0" + hour1;
		}
		minute1 = to_string(rand() % 60);
		if (minute1.size() == 1)
		{
			minute1 = "0" + minute1;
		}

		if (day1.size() == 1)
		{
			day1 = "0" + day1;
		}
		if (month1.size() == 1)
		{
			month1 = "0" + month1;
		}

		if (day1 == "31" && hour1 > "15")
		{
			day2 = to_string((31 + rand() % 2) % 32); //1 || 31
			if (day2 == "0")
			{
				day2 = "1";
				month2 = to_string(stoi(month1) + 1);
			}
			hour2 = to_string((15 + rand() % 24) % 24);
			minute2 = to_string(rand() % 60);
		}
		else
		{

			month2 = month1;
			if (hour1 > "15")
			{
				hour2 = to_string((stoi(hour1) + rand() % 24) % 24);
				day2 = to_string(stoi(day1) + 1);
			}
			else
			{
				day2 = day1;
				hour2 = to_string(stoi(hour1) + rand() % 9);
			}
			minute2 = to_string(rand() % 60);
			if (hour2.size() == 1)
			{
				hour2 = "0" + hour2;
			}
			if (minute2.size() == 1)
			{
				minute2 = "0" + minute2;
			}
			if (day2.size() == 1)
			{
				day2 = "0" + day2;
			}
			if (month2.size() == 1)
			{
				month2 = "0" + month2;
			}
		}

		date_d = year1 + "." + month1 + "." + day1 + "_" + hour1 + ":" + minute1;
		date_a = year2 + "." + month2 + "." + day2 + "_" + hour2 + ":" + minute2;


		free_site = rand() / int(500);
		occupied_site = rand() / int(500);


		price = 998 + rand() % 15000;
		company = aviacompany[rand() % size_aviacompany];
		string j = to_string(i + 1);
		if (i + 1 >= 1 && i + 1 < 10)
		{
			j = "00000" + j;
		}

		if (i + 1 >= 10 && i + 1 < 100)
		{
			j = "0000" + j;
		}

		if (i + 1 >= 100 && i + 1 < 1000)
		{
			j = "000" + j;
		}

		if (i + 1 >= 1000 && i + 1 < 10000)
		{
			j = "00" + j;
		}

		if (i + 1 >= 10000 && i + 1 < 100000)
		{
			j = "0" + j;
		}
		A[i].number = j.c_str();
		A[i].str_city_ar = departure.c_str();
		A[i].str_city_dep = arrival.c_str();
		A[i].str_company = company.c_str();
		A[i].str_date_ar = date_d;
		A[i].str_date_dep = date_a;
		A[i].str_free_site = free_site;
		A[i].str_occupied_site = occupied_site;
		A[i].str_price = price;
		A[i].status = status;
		std::fprintf(f, "%s. %s %s %s %s %d %d %d %s %s\n\n", j.c_str(), departure.c_str(), arrival.c_str(), date_d.c_str(), date_a.c_str(), free_site, occupied_site, price, company.c_str(), status.c_str());
		//f << i + 1 << departure.c_str() << arrival.c_str() << date_d.c_str() << date_a.c_str() << free_site << occupied_site << price<< company.c_str();
	}

}

void Search(vector<int>& str_res, Avia* A, string x)
{
	setlocale(LC_ALL, "Russian");
	//vector<string> str_res;
	int i = 0;
	string number = "", city_ar = "", city_dep = "", date_ar = "", date_der = "", free_site = "", occupied_site = "", price = "", company = "";

	stringstream tmp;        //Создание потоковой переменной
	tmp << x;
	tmp >> number;
	tmp >> city_ar;
	tmp >> city_dep;
	tmp >> date_ar;
	tmp >> date_der;
	tmp >> free_site;
	tmp >> occupied_site;
	tmp >> price;
	tmp >> company;
	//tmp >> status;
	for (int i = 0; i < company.size(); ++i)
	{
		if (company[i] == NULL)
		{
			company.erase(i, company.size());
			i = company.size();
		}
	}
	string date_from = "-"; //+3day
	string date_to = "-";//-3day
	if (date_ar != "-" || date_der != "-") {
		string date;
		if (date_ar != "-")
		{
			date = date_ar;
		}
		else {
			date = date_der;
		}
		stringstream ss;
		ss << date[5] << date[6];
		string m = ss.str();
		//cout << m << " ";
		int month_from = atoi(m.c_str());
		int month_to = atoi(m.c_str());

		stringstream sss;
		sss << date[8] << date[9];
		string d = sss.str();
		//cout << d << endl;

		int day_from = atoi(d.c_str());
		int day_to = atoi(d.c_str());
		if (day_from > 28)
		{
			day_from = (day_from + 3) % 31;
			month_from += 1;
		}
		else {
			day_from = day_from + 3;
		}

		if (day_to < 3)
		{
			day_to = (day_to - 3) % 31;
			month_to -= 1;
		}
		else {
			day_to = (day_to - 3);
		}

		string str_day_from = to_string(day_from);
		string str_month_from = to_string(month_from);

		string str_day_to = to_string(day_to);
		string str_month_to = to_string(month_to);

		if (str_day_from.size() == 1)
		{
			str_day_from = "0" + str_day_from;
		}
		if (str_month_from.size() == 1)
		{
			str_month_from = "0" + str_month_from;
		}

		if (str_day_to.size() == 1)
		{
			str_day_to = "0" + str_day_to;
		}
		if (str_month_to.size() == 1)
		{
			str_month_to = "0" + str_month_to;
		}

		stringstream hh;
		hh << date[11] << date[12];
		string h = hh.str();

		stringstream mm;
		mm << date[14] << date[15];
		string mon = mm.str();

		date_from = "2022." + str_month_from + "." + str_day_from + "_" + h + ":" + mon;


		date_to = "2022." + str_month_to + "." + str_day_to + "_" + h + ":" + mon;
		//cout << month_from << " " << month_from << endl;
			//string date_ar_from = //+-3 day
	}

	//	string free_site_str  
	string res = number + ". " + city_ar + " " + city_dep + " " + date_ar + " " + date_der + " " + free_site + " " + occupied_site + " " + price + " " + company + " ";
	std::cout << res << endl << endl;
	string str;

	int k = 0;
	for (int i = 0; i < 10000; i++) {
		if ((A[i].number == number) || (number == "-")) {
			++k;
		}
		if ((A[i].str_city_ar == city_ar) || (city_ar == "-")) {
			++k;
		}
		if ((A[i].str_city_dep == city_dep) || (city_dep == "-")) {
			++k;
		}
		if ((A[i].str_company == company) || (company == "-")) {
			++k;
		}
		if (((A[i].str_free_site == atoi(free_site.c_str())) || (free_site == "-")) && A[i].str_free_site != 0) {
			++k;
		}

		if ((A[i].str_occupied_site == atoi(occupied_site.c_str())) || (occupied_site == "-")) {
			++k;
		}
		if (((atoi(price.c_str()) <= A[i].str_price + 500) && (A[i].str_price - 500 <= atoi(price.c_str()))) || (price == "-")) {
			++k;
		}

		if (((date_to.c_str() <= A[i].str_date_ar) && (A[i].str_date_ar <= date_from.c_str())) || (date_from == "-")) {
			++k;
		}
		if (A[i].status == "Active")
		{
			++k;
		}
		if (k == 9) {
			//std::cout << A[i];
			str_res.push_back(i);
		}
		k = 0;
	}




	//return str_res;


}

void Add(Avia* A, string s, int size) {
	string number, city_ar, city_dep, date_ar, date_der, free_site, occupied_site, price, company, index;
	stringstream tmp;        //Создание потоковой переменной
	tmp << s;
	tmp >> number;
	tmp >> city_ar;
	tmp >> city_dep;
	tmp >> date_ar;
	tmp >> date_der;
	tmp >> free_site;
	tmp >> occupied_site;
	tmp >> price;
	tmp >> company;
	tmp >> index;
	//--i;//
	int i = atoi(index.c_str());
	--i;
	//	cout << i;
	int n = size - i;
	//int m_site = i;
	Avia* B = new Avia[n];

	copy(&A[i], &A[size - 1], &B[0]);	//-1
	A[i].number = number.c_str();
	A[i].str_city_ar = city_ar.c_str();
	A[i].str_city_dep = city_dep.c_str();
	A[i].str_company = company.c_str();
	A[i].str_date_ar = date_ar;
	A[i].str_date_dep = date_der;
	A[i].str_free_site = atoi(free_site.c_str());
	A[i].str_occupied_site = atoi(occupied_site.c_str());
	A[i].str_price = atoi(price.c_str());
	A[i].status = "Active";
	copy(&B[0], &B[size - i - 1], &A[i + 1]);//+1 -> -1
	cout << "Add - OK" << endl;

}

void Delete(Avia* A, string s, int size)
{

	string number;
	stringstream tmp;        //Создание потоковой переменной
	tmp << s;
	tmp >> number;

	int c = 0;
	number.erase(number.end() - 1);
	for (int i = 0; i < size; ++i)
	{
		if (A[i].number == number)
		{
			A[i].status = "Deleted";
			c = i;
			i = size;
			cout << "Deleted - OK" << endl;
		}
	}
	//for (int i = c; i < size-1; ++i)
	//{
	//	A[i] = A[i + 1];
	//}



}

int Buy(Avia* A, string s, int size)
{
	string number, city_ar, city_dep, date_ar, site;
	stringstream tmp;        //Создание потоковой переменной
	tmp << s;
	tmp >> number;
	tmp >> city_ar;
	tmp >> city_dep;
	tmp >> date_ar;
	tmp >> site;

	int c = 0;
	int flag = 0;
	for (int i = 0; i < size; ++i)
	{
		if (A[i].number == number)
		{
			if (A[i].status == "Active") {
				A[i].status = "Booking";
				if (A[i].str_free_site >= atoi(site.c_str()))
				{
					A[i].str_free_site = A[i].str_free_site - atoi(site.c_str());
					cout << "Buy - OK" << endl;
					flag = 1;
				}
				else {
					cout << "It is impossible to buy a ticket, there are not enough seats" << endl;
					flag = -1;
				}
				A[i].status = "Active";
			}
			else {
				cout << "It is impossible to buy a ticket, there is no such flight or it is booked" << endl;
				 flag = -1;
			}
		}
	}
	return flag;
}



