
//#include "stdafx.h"
#pragma warning(suppress : 4996)
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <clocale>
#include<fstream>
#include<string>


#include <ctype.h>

#include <io.h>
#include <fcntl.h>
#include <stdint.h>

#pragma once
#include <math.h>
#include <fstream>
//#include <ifstream>
#include <sstream>
#include <string>
#include <vector>
#include<cstdlib>


// Определимся с портом, адресом сервера и другими константами.
// В данном случае берем произвольный порт и адрес обратной связи
// (тестируем на одной машине).
#define  SERVER_PORT     5555
//#define  SERVER_PORT     80
#define  SERVER_NAME    "127.0.0.1"
//#define  BUFLEN         4096 
#define  BUFLEN         256 

using namespace std;

// Две вспомогательные функции для чтения/записи (см. ниже)
int  writeToServer(int fd, string str);
int  readFromServer(int fd);

int  main(int argc, char** argv)
{
    
   ifstream fin("input1.txt");

   
    vector<string> str_script;
    string script;
    while (getline(fin, script))
    {
        str_script.push_back(script);
       // std::cout << script;
    }
    for (string t : str_script)
    {
       // fprintf(stdout, "%s\n", t);
        cout << t << endl;
    }
    //vector<string> str_script = {};
    SetConsoleCP(1251);
    setlocale(LC_CTYPE, "rus");
    setlocale(LC_ALL, "Russian");
    // по идее можно обратиться куда угодно 
    const char* serverName;
    serverName = (argc < 2) ? "127.0.0.1" : argv[1];
    printf("connecting to %s\n", serverName);

    int err;
    int sock;
    struct sockaddr_in server_addr;
    struct hostent* hostinfo;

    // инициализация windows sockets
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return -1;
    }

    // Получаем информацию о сервере по его DNS имени
    // или точечной нотации IP адреса.
    hostinfo = gethostbyname(serverName);
    char sss[256];
    printf("host %s\n", inet_ntop(AF_INET, hostinfo->h_addr, sss, 256));
    if (hostinfo == NULL) {
        fprintf(stderr, "Unknown host %s.\n", SERVER_NAME);
        exit(EXIT_FAILURE);
    }
    // можно было бы использовать GetAddrInfo()

    // Заполняем адресную структуру для последующего
    // использования при установлении соединения
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr = *(struct in_addr*)hostinfo->h_addr;

    //unsigned int iaddr;
    //inet_pton(AF_INET, serverName, &iaddr);
    //server_addr.sin_addr.s_addr = iaddr;


    // Создаем TCP сокет.
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Client: socket was not created");
        exit(EXIT_FAILURE);
    }

    // Устанавливаем соединение с сервером
    err = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (err < 0) {
        perror("Client:  connect failure");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Connection is ready\n");
    //string str1 = "Search - Nizhnekamsk Sietl - - - - - -";
    
    // Обмениваемся данными
    while (1) {
        for (string str1 : str_script) {
           // cout << "OK";
            if (writeToServer(sock, str1) < 0) break;
            if (readFromServer(sock) < 0) break;
        }
    }
    fprintf(stdout, "The end\n");

    // Закрываем socket
    closesocket(sock);
    WSACleanup();
    exit(EXIT_SUCCESS);
}



int  writeToServer(int fd, string str)
{
#define _CRT_SECURE_NO_WARNINGS
    SetConsoleCP(1251);
    setlocale(LC_CTYPE, "rus");
    setlocale(LC_ALL, "Russian");
    int   nbytes;
    char  buf[BUFLEN];

 
    int j = 0;
    for (auto ch : str)
    {
        buf[j] = ch;
        ++j;
    }
    buf[j] = NULL;
    //std::cout << buf<<endl;
   // buf[strlen(buf) - 1] = 0;
   // std::cout << buf << endl;
    nbytes = send(fd, buf, strlen(buf) + 1, 0);
    printf("send %d bytes\n", nbytes);
    
    if (strstr(buf, "stop")) return -1;
    return 0;
}


int  readFromServer(int fd)
{
    SetConsoleCP(1251);
    setlocale(LC_CTYPE, "rus");
    setlocale(LC_ALL, "Russian");
    int   nbytes;
    char  buf[BUFLEN];

    nbytes = recv(fd, buf, BUFLEN, 0);
    if (nbytes < 0) {
        // ошибка чтения
        perror("read");
        return -1;
    }
    else if (nbytes == 0) {
        // нет данных для чтения
        fprintf(stderr, "Client: no message\n");
    }
    else {
        // ответ успешно прочитан
        
        fprintf(stdout, "Server's replay: %s\n", buf);
    }
    return 0;
}



