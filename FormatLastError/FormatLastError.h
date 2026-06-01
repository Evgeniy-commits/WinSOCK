#pragma once

#ifndef WIN32_LEAN_AND_MEAN		//Для добавления <Windows.h> и <iphplapi.h>
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include<Windows.h>
#include<cstdio>
#include<iostream>
#include<fstream>
#include<WinSock2.h>
#include <WS2tcpip.h>
#include <ctime>
#include <chrono>

#pragma comment(lib, "WS2_32.lib")

using namespace std;

LPSTR FormatLastError(DWORD dwError, CHAR szBuffer[]);
VOID clientInfo(SOCKET client_socket, sockaddr_in& clientAddr, INT szClientAddr, ofstream& logS);