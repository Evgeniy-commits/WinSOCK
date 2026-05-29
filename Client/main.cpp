//WinSOCK (Windows Sockets)
#define _CRT_SECURE_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN		//ƒл€ добавлени€ <Windows.h> и <iphplapi.h>
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


#include<iostream>
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>

#include<FormatLastError.h>

using namespace std;

#pragma comment(lib, "WS2_32.lib") 
#pragma comment(lib, "FormatLastError.lib") 

#define PORT	"27015"
#define BUFFER_LENGTH	1500

void main()
{
	setlocale(LC_ALL, "");
	cout << "CLIENT" << endl;
	CHAR szError[256] = {};

	//INIT WinSOCK
	WSADATA wsaData;	//экземпл€р структуры
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if ( iResult != 0)
	{
		cout << "WSAStartup failed: " << iResult << endl;
		return;
	}

	//2)«адаем параметры подключени€: IP - адрес сервера и порт
	struct addrinfo hints;
	struct addrinfo* result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// уда подключаемс€
	iResult = getaddrinfo("127.0.0.1", PORT, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed: " << iResult << endl;
		WSACleanup();
		return;
	}

	//3) —оздаем сокет дл€ клиента
	SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (connect_socket == INVALID_SOCKET)
	{
		cout << FormatLastError(WSAGetLastError(), szError) << endl;
		cout << "Socket creation error: " << WSAGetLastError() << endl;

		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//4) ѕодключаемс€ к серверу
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

	//5) ќ“правка и получение данных
		CHAR sendbuffer[BUFFER_LENGTH] = "Hello Server";
		CHAR recvbuffer[BUFFER_LENGTH] = {};

		iResult = send(connect_socket, sendbuffer, strlen(sendbuffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			cout << FormatLastError(WSAGetLastError(), szError) << endl;
			cout << "Send failed:\t" << WSAGetLastError() << endl;
			closesocket(connect_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}
		cout << "Bytes sent: " << iResult << endl;
			
		do
		{
			iResult = recv(connect_socket, recvbuffer, BUFFER_LENGTH, 0);
			if (iResult > 0) cout << recvbuffer << "(" << iResult << " Bytes)" << endl;
			else if (result == 0) cout << "Connection closed" << endl;
			else cout << FormatLastError(WSAGetLastError(), szError) << endl;
				//cout << "Receive failed\t" << WSAGetLastError() << endl;
		} while (iResult > 0);

		iResult = shutdown(connect_socket, SD_BOTH);
		if (iResult == SOCKET_ERROR)
		{
			cout << FormatLastError(WSAGetLastError(), szError) << endl;
			cout << "Shutdown failed: " << WSAGetLastError() << endl;
			closesocket(connect_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}
}

//FORMAT_MESSAGE_ALLOCATE_BUFFER Ч просит систему выделить пам€ть дл€ буфера автоматически.
// ¬ этом случае lpMsgBuf должен быть указателем на void*, 
// куда будет записан адрес выделенной пам€ти.
//
//FORMAT_MESSAGE_FROM_SYSTEM Ч указывает, что нужно искать описание ошибки в системных
// сообщени€х Windows.
//
//FORMAT_MESSAGE_IGNORE_INSERTS Ч игнорирует Ђвставкиї(placeholders) в шаблоне сообщени€.
// Ќекоторые системные сообщени€ содержат места дл€ подстановки данных(например, им€ файла), 
// но дл€ кодов ошибок это обычно не нужно.

//»сточник сообщени€(NULL) Ч так как используетс€ флаг FORMAT_MESSAGE_FROM_SYSTEM, 
// источник не требуетс€.—истема сама найдЄт сообщение в системных ресурсах.
//
// од ошибки(errorCode) Ч входной параметр, код ошибки, дл€ которого нужно получить описание
// Ќапример, это может быть код ошибки Winsock(WSAGetLastError()).
//
//язык(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)) Ч задаЄт €зык дл€ сообщени€.
// LANG_NEUTRAL означает Ђлюбой подход€щий €зыкї, система выберет наиболее подход€щий
// (обычно текущий системный €зык).
//
//Ѕуфер дл€ вывода((LPTSTR)&lpMBuf) Ч адрес указател€, куда будет записан адрес выделенного буфера.
// ѕри использовании флага FORMAT_MESSAGE_ALLOCATE_BUFFER система выделит пам€ть и запишет адрес
// в lpBuf.
//
//–азмер буфера(0) Ч не используетс€ при FORMAT_MESSAGE_ALLOCATE_BUFFER, 
// так как система сама определ€ет нужный размер.
//
//јргументы(NULL) Ч не используютс€ при FORMAT_MESSAGE_IGNORE_INSERTS.