//WinSOCK (Windows Sockets)
#define _CRT_SECURE_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN		//Для добавления <Windows.h> и <iphplapi.h>
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


#include<iostream>
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>
#include<string>
#include<chrono>
#include<thread>
#include<atomic>
#include<mutex>

#include<FormatLastError.h>
#include<Messages.h>

using namespace std;

#pragma comment(lib, "WS2_32.lib") 
#pragma comment(lib, "FormatLastError.lib") 

#define PORT	"27015"
#define BUFFER_LENGTH	1500

VOID receiveThread(SOCKET connect_socket);
mutex outputMutex;
string IdenMess(SOCKET connect_socket);

void main()
{
	setlocale(LC_ALL, "");
	cout << "CLIENT" << endl;

	CHAR szError[256] = {};

	//INIT WinSOCK
	WSADATA wsaData;	//экземпляр структуры
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if ( iResult != 0)
	{
		cout << "WSAStartup failed: " << iResult << endl;
		return;
	}

	//2)Задаем параметры подключения: IP - адрес сервера и порт
	struct addrinfo hints;
	struct addrinfo* result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//Куда подключаемся
	iResult = getaddrinfo("127.0.0.1", PORT, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed: " << iResult << endl;
		WSACleanup();
		return;
	}

	//3) Создаем сокет для клиента
	SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (connect_socket == INVALID_SOCKET)
	{
		cout << FormatLastError(WSAGetLastError(), szError) << endl;
		cout << "Socket creation error: " << WSAGetLastError() << endl;

		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//4) Подключаемся к серверу
	iResult = connect(connect_socket, result->ai_addr, result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			cout << FormatLastError(WSAGetLastError(), szError) << endl;
			cout << "Unable to connect to Server" << endl;

			closesocket(connect_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}


	// Запускаем поток приёма сообщений
	thread receiver(receiveThread, connect_socket);

	//5.3) ОТправка и получение данных
	CHAR sendbuffer[BUFFER_LENGTH] = { "\tHello World" };
	//5.1) Отправляем имя имя или инфо на сервер

	string idenMess = IdenMess(connect_socket);

	do
	{
		ZeroMemory(sendbuffer, BUFFER_LENGTH);
		string message = "";
		SetConsoleCP(1251);
		cout << idenMess << "> ";
		cin.getline(sendbuffer, BUFFER_LENGTH);
		SetConsoleCP(866);
		CHAR recvbuffer[BUFFER_LENGTH] = {};
		message = idenMess + "> " + sendbuffer;
				
		iResult = send(connect_socket, message.c_str(), message.length(), 0);
		if (iResult == SOCKET_ERROR)
		{
			cout << FormatLastError(WSAGetLastError(), szError) << endl;
			cout << "Send failed:\t" << WSAGetLastError() << endl;
			closesocket(connect_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}
		
		//memset(recvbuffer, 0, BUFFER_LENGTH);
		//iResult = recv(connect_socket, recvbuffer, BUFFER_LENGTH, 0);
		//cout << recvbuffer << endl;
		if (iResult == 0) cout << "Connection closed" << endl;
		
		if (strcmp(recvbuffer, DECLINE_MESSAGE) == 0)
		{
			system("PAUSE");
			break;
		}
	} while (strcmp(sendbuffer, "exit") != 0);

	
	iResult = shutdown(connect_socket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		cout << FormatLastError(WSAGetLastError(), szError) << endl;
		cout << "Shutdown failed: " << WSAGetLastError() << endl;
	}

	receiver.join();

	closesocket(connect_socket);
	freeaddrinfo(result);
	WSACleanup();
}

string IdenMess(SOCKET connect_socket)
{
	//Ввод имени пользователя
	string nick;
	cout << "Введите свое имя: ";
	SetConsoleCP(1251);
	getline(cin, nick);
	SetConsoleCP(866);
	//cout << endl;

	string idenMess;
	if (!nick.empty())
	{
		idenMess = nick;
	}
	else
	{
		sockaddr_in locAddr;
		INT addrLen = sizeof(locAddr);
		if (getsockname(connect_socket, (sockaddr*)&locAddr, &addrLen) != SOCKET_ERROR)
		{
			CHAR locIP[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &locAddr.sin_addr, locIP, INET_ADDRSTRLEN);
			INT locPort = ntohs(locAddr.sin_port);
			idenMess = "IP: " + string(locIP) + " : " + to_string(locPort);
		}
		else
		{
			idenMess = "Мы не занем что это!!!";
		}
	}
	return idenMess;
}

// Поток для приёма сообщений от сервера
VOID receiveThread(SOCKET connect_socket) 
{
	CHAR recvbuffer[BUFFER_LENGTH];
	int iResult;

	while (true) {
		ZeroMemory(recvbuffer, BUFFER_LENGTH);
		iResult = recv(connect_socket, recvbuffer, BUFFER_LENGTH - 1, 0);

		if (iResult > 0) 
		{
			lock_guard<mutex> lock(outputMutex);
			cout << "[SER]" << recvbuffer << endl;
			//std::cout << (nick.empty() ? "You" : nick) << "> ";
		}
		else if (iResult == 0) {
			cout << "\nConnection closed by server" << endl;
			break;
		}
		else {
			DWORD error = WSAGetLastError();
			CHAR errorMsg[256] = {};
			cout << "\nReceive error: " << FormatLastError(error, errorMsg) << endl;
			break;
		}
		this_thread::sleep_for(chrono::milliseconds(10));
	}
}

//FORMAT_MESSAGE_ALLOCATE_BUFFER — просит систему выделить память для буфера автоматически.
// В этом случае lpMsgBuf должен быть указателем на void*, 
// куда будет записан адрес выделенной памяти.
//
//FORMAT_MESSAGE_FROM_SYSTEM — указывает, что нужно искать описание ошибки в системных
// сообщениях Windows.
//
//FORMAT_MESSAGE_IGNORE_INSERTS — игнорирует «вставки»(placeholders) в шаблоне сообщения.
// Некоторые системные сообщения содержат места для подстановки данных(например, имя файла), 
// но для кодов ошибок это обычно не нужно.

//Источник сообщения(NULL) — так как используется флаг FORMAT_MESSAGE_FROM_SYSTEM, 
// источник не требуется.Система сама найдёт сообщение в системных ресурсах.
//
//Код ошибки(errorCode) — входной параметр, код ошибки, для которого нужно получить описание
// Например, это может быть код ошибки Winsock(WSAGetLastError()).
//
//Язык(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)) — задаёт язык для сообщения.
// LANG_NEUTRAL означает «любой подходящий язык», система выберет наиболее подходящий
// (обычно текущий системный язык).
//
//Буфер для вывода((LPTSTR)&lpMBuf) — адрес указателя, куда будет записан адрес выделенного буфера.
// При использовании флага FORMAT_MESSAGE_ALLOCATE_BUFFER система выделит память и запишет адрес
// в lpBuf.
//
//Размер буфера(0) — не используется при FORMAT_MESSAGE_ALLOCATE_BUFFER, 
// так как система сама определяет нужный размер.
//
//Аргументы(NULL) — не используются при FORMAT_MESSAGE_IGNORE_INSERTS.
