//WinSOCK (Windows Sockets)
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#ifndef WIN32_LEAN_AND_MEAN		//Для добавления <Windows.h> и <iphplapi.h>
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


#include<iostream>
#include<fstream>
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>

#include<FormatLastError.h>
#include<Messages.h>

using namespace std;

#pragma comment(lib, "WS2_32.lib") //Для добавления <WinSock2.h> <WS2tcpip.h>
#pragma comment(lib, "FormatLastError.lib") 

#define PORT			"27015"
#define BUFFER_LENGTH	1500
#define MAX_CONNECTIONS	3

SOCKET sockets[MAX_CONNECTIONS] = {};
DWORD dwThreadIDs[MAX_CONNECTIONS] = {};
HANDLE hThreads[MAX_CONNECTIONS] = {};

//struct ClientParameters
//{
//	SOCKET client_socket;
//	sockaddr_in client_address;
//};

VOID ClientHandle(LPVOID lpfreeSl);

void main()
{
	setlocale(LC_ALL, "");
	cout << "SERVER" << endl;
	DWORD dwError = 0;
	CHAR szError[256] = {};
	
	//INIT WinSOCK
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	dwError = WSAGetLastError();
	if (iResult != 0)
	{
		cout << FormatLastError(dwError, szError) << endl;
		cout << "WSAStartup failed: " << iResult << endl;
		return;
	}
	
	//2) Параметры подключения
	addrinfo hints;
	addrinfo* result;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	dwError = WSAGetLastError();
	if (iResult != 0)
	{
		cout << "getaddrinfo() failed: " << iResult << endl;
		WSACleanup();
		return;
	}

	//3)создаем сокет сервера, который он будет постоянно слушать "LISTENING"
	SOCKET listen_socket =
		socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	dwError = WSAGetLastError();
	if (listen_socket == INVALID_SOCKET)
	{
		cout << FormatLastError(dwError, szError) << endl;
		cout << "Listen socket error: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//4)Bind socket
	iResult = bind(listen_socket, result->ai_addr, result->ai_addrlen);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR)
	{
		cout << FormatLastError(dwError, szError) << endl;
		cout << "Bind failed with error: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
	freeaddrinfo(result);

	//5) Запустить прослушивание сокета
	if (listen(listen_socket, MAX_CONNECTIONS) == SOCKET_ERROR)
	{
		dwError = WSAGetLastError();
		cout << FormatLastError(dwError, szError) << endl;
		cout << "Listen failed with error: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//6) Обработка соединений от клиентов:
	do
	{
		sockaddr_in client_address;
		int client_addrlen = sizeof(client_address);
		client_address.sin_family = AF_INET;
		SOCKET client_socket = accept(listen_socket, (SOCKADDR*)&client_address, &client_addrlen);
		dwError = WSAGetLastError();
		if (client_socket == INVALID_SOCKET)
		{
			cout << FormatLastError(dwError, szError) << endl;
			cout << "Accept failed with error: " << WSAGetLastError() << endl;
		}

		//6.1) Получаем информацию о сокете клиента
		CHAR* clientIP = inet_ntoa(client_address.sin_addr);
		cout << clientIP << endl;

		int clientPort = ntohs(client_address.sin_port);
		cout << clientPort << endl;


		//6.2 Запускаем взаимодействие с клиентом
		//ClientHandle(client_socket);

		//Ищем свободный слот
		int freeSl = -1;
		for (int j = 0; j < MAX_CONNECTIONS; j++)
		{
			if (sockets[j] == INVALID_SOCKET || hThreads[j] == NULL)
			{
				freeSl = j;
				break;
			}
		}

		if (freeSl != -1)
		{
			sockets[freeSl] = client_socket;
			hThreads[freeSl] = CreateThread
			(
				NULL,			//Security attributes
				0,				//Stack size
				(LPTHREAD_START_ROUTINE)ClientHandle,	//указатель на ф-ю, кот. будет вып-ся в потоке
				(LPVOID)freeSl,
				0,
				&dwThreadIDs[freeSl]
			);
		}
		else
		{
			CHAR recv_buffer[BUFFER_LENGTH] = {};
			iResult = recv(client_socket, recv_buffer, BUFFER_LENGTH, NULL);
			/*if (iResult != 0)
			{
				FormatLastError(WSAGetLastError(), szError);
				cout << szError << endl;
			}
			else*/ cout << recv_buffer << endl;
			//CHAR szDeclainMessage[] = ;
			iResult = send(client_socket, DECLINE_MESSAGE, strlen(DECLINE_MESSAGE), NULL);
			shutdown(client_socket, SD_BOTH);
			closesocket(client_socket);
		}
	} while (true);


	/*iResult = shutdown(listen_socket, SD_BOTH);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR) 
		cout << "Client shutdown failed with error: " << FormatLastError(dwError, szError) << endl;*/
	
	closesocket(listen_socket);
	WSACleanup();
}

VOID ClientHandle(LPVOID lpfreeSl)
{ 
	INT i = (INT) lpfreeSl;	//счетчик клиентов
	SOCKET client_socket = sockets[i];

	sockaddr_in client_address;
	client_address.sin_family = AF_INET;
	INT client_addresslen = sizeof(client_address);
	getpeername(client_socket, (sockaddr*)&client_address, &client_addresslen);
	CHAR szClient_address[32] = {};
	sprintf(szClient_address, "%s : %d - ", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

	cout << "Client connected" << szClient_address << "SOCKET\t" << client_socket << endl;
	INT iResult = 0;
	DWORD dwError;
	CHAR szError[256] = {};

	//7) Получение и отправка данных
	INT iSendResult = 0;

	ofstream logS("server_log", ios::app);
	if (!logS.is_open())
	{
		cout << "Faild to open log" << endl;
	}

	do
	{
		CHAR sendbuffer[BUFFER_LENGTH] = {};
		CHAR recvbuffer[BUFFER_LENGTH] = {};
		memset(recvbuffer, 0, BUFFER_LENGTH);

		iResult = recv(client_socket, recvbuffer, BUFFER_LENGTH, 0);
		dwError = WSAGetLastError();
		if (iResult > 0)
		{
			cout << recvbuffer << "(" << strlen(recvbuffer) << " Bytes)" << endl;
			logS << "RECV: " << recvbuffer << "(" << iResult << " Bytes)" << endl;
			iSendResult = send(client_socket, recvbuffer, strlen(recvbuffer), 0);
			dwError = WSAGetLastError();
			if (iSendResult == SOCKET_ERROR)
			{
				cout << FormatLastError(dwError, szError) << endl;
				cout << "Send failed with error: " << WSAGetLastError() << endl;
				logS << "ERROR: Send failed with error: " << dwError << endl;
				closesocket(client_socket);
			}
			else
			{
				cout << "Bytes sent: " << iSendResult << endl;
				logS << "SEND: Bytes sent: " << recvbuffer << "(" << iSendResult << " bytes)" << endl;
			}
		}
		else if (iResult == 0)
		{
			//i--;
			cout << "Connection closing..." << endl;
			cout << "Client unconnected" << szClient_address << "SOCKET\t" << client_socket << endl;
			logS << "INFO: Connection closing..." << endl;
			sockets[i] = INVALID_SOCKET;
			hThreads[i] = NULL;
			dwThreadIDs[i] = 0;
		}
		else
		{
			cout << FormatLastError(dwError, szError) << endl;
			cout << "Receive failed with error: " << WSAGetLastError() << endl;
			logS << "Receive failed with error: " << dwError << endl;
			closesocket(client_socket);
		}
	} while (iResult > 0);
	logS.close();

	iResult = shutdown(client_socket, SD_BOTH);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR)
		cout << "Client shutdown failed with error: " << FormatLastError(dwError, szError) << endl;

	closesocket(client_socket);
}