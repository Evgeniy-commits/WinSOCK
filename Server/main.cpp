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

INT g_ActiveClient = 0;
SOCKET sockets[MAX_CONNECTIONS] = {};
DWORD dwThreadIDs[MAX_CONNECTIONS] = {};
HANDLE hThreads[MAX_CONNECTIONS] = {};

//struct ClientParameters
//{
//	SOCKET client_socket;
//	sockaddr_in client_address;
//};

VOID ClientHandle(LPVOID lpfreeSl);
//VOID Release(SOCKET client_socket);
VOID ShowActiveClients();

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
		ShowActiveClients();
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

		if (g_ActiveClient < MAX_CONNECTIONS)
		{
			sockets[g_ActiveClient] = client_socket;
			cout << client_socket << "\t" << sockets[g_ActiveClient] << endl;
			hThreads[g_ActiveClient] = CreateThread
			(
				NULL,			//Security attributes
				0,				//Stack size
				(LPTHREAD_START_ROUTINE)ClientHandle,	//указатель на ф-ю, кот. будет вып-ся в потоке
				(LPVOID)g_ActiveClient,
				0,
				&dwThreadIDs[g_ActiveClient]
			);
			g_ActiveClient++;
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
	WaitForMultipleObjects(MAX_CONNECTIONS, hThreads, TRUE, INFINITE);

	/*iResult = shutdown(listen_socket, SD_BOTH);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR) 
		cout << "Client shutdown failed with error: " << FormatLastError(dwError, szError) << endl;*/
	
	closesocket(listen_socket);
	WSACleanup();
}

INT GetSlotIndex(DWORD dwID)
{
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (dwThreadIDs[i] == dwID) return i;
	}
}

VOID Shift(INT start)
{
	for (INT i = 0; i < MAX_CONNECTIONS; i++)
	{
		sockets[i] = sockets[i + 1];
		hThreads[i] = hThreads[i + 1];
		dwThreadIDs[i] = dwThreadIDs[i + 1];
	}
	sockets[MAX_CONNECTIONS - 1] = NULL;
	hThreads[MAX_CONNECTIONS - 1] = NULL;
	dwThreadIDs[MAX_CONNECTIONS - 1] = NULL;
	g_ActiveClient--;
}

VOID ClientHandle(LPVOID g_ActiveClient)
{ 
	INT i = (INT)g_ActiveClient;	//счетчик клиентов
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
			cout << "Connection closing..." << endl;
			cout << "Client unconnected" << szClient_address << "SOCKET\t" << client_socket << endl;
			logS << "INFO: Connection closing..." << endl;
			
		}
		else
		{
			cout << FormatLastError(dwError, szError) << endl;
			cout << "Receive failed with error: " << WSAGetLastError() << endl;
			logS << "Receive failed with error: " << dwError << endl;
			closesocket(client_socket);
		}
	} while (iResult > 0);
	DWORD dwID = GetCurrentThreadId();
	Shift(GetSlotIndex(dwID));
	cout << szClient_address << " left" << endl;
	logS.close();

	iResult = shutdown(client_socket, SD_BOTH);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR)
		cout << "Client shutdown failed with error: " << FormatLastError(dwError, szError) << endl;

	closesocket(client_socket);
	ShowActiveClients();
	ExitThread(0);
}

//VOID Release(SOCKET client_socket)
//{
//	for (int i = 0; i < MAX_CONNECTIONS; i++)
//	{
//		if (client_socket == sockets[i])
//		{
//			sockets[i] = INVALID_SOCKET;
//			//hThreads[i] = NULL;
//			//dwThreadIDs[i] = 0;
//			for (int j = i; sockets[j] || j < MAX_CONNECTIONS - 1; j++)
//			{
//				sockets[j] = sockets[j + 1];
//				hThreads[j] = hThreads[j + 1];
//				dwThreadIDs[j] = dwThreadIDs[j + 1];
//			}
//		}
//	}
//	g_ActiveClient--;
//	ShowActiveClients();
//}
//
VOID ShowActiveClients()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hConsole, &info);
	COORD cursor = { 1, 25 };
	SetConsoleCursorPosition(hConsole, cursor);
	cout << "Количество подключений: " << g_ActiveClient;
	SetConsoleCursorPosition(hConsole, info.dwCursorPosition);
}