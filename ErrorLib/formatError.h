#pragma once
#ifndef WIN32_LEAN_AND_MEAN		
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


#include<iostream>
#include<string>
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>

using namespace std;

#pragma comment(lib, "WS2_32.lib")

void FormatWSALastError();