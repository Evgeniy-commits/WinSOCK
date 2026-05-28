#include "pch.h"
#include "framework.h"

#include"formatError.h"
#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "WS2_32.lib")

void FormatWSALastError()
{
	DWORD errorCode = WSAGetLastError();

	wchar_t errorStr[1024] = {};
	DWORD errorStrSize = sizeof(errorStr) / sizeof(wchar_t) - 2;

	/*if (GetIpErrorString(errorCode, errorStr, &errorStrSize) == NO_ERROR)
	{
		wcout << L"Error code: " << errorCode << endl;
		wcout << L"Error description: " << errorStr << endl;
	}
	else
	{*/
		LPVOID lpMsgBuf = nullptr;

		FormatMessage
		(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			nullptr
		);
		if(lpMsgBuf)
		{
			cout << "Error code: " << errorCode << endl;
			cout << "Error description: " << (char*)lpMsgBuf << endl;
			LocalFree(lpMsgBuf);
		}
		else
		{
			cout << "Error code: " << errorCode << endl;
			cout << "Error description: Unknow error" << endl;
		}
	//}
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
//Ѕуфер дл€ вывода((LPTSTR)&lpMsgBuf) Ч адрес указател€, куда будет записан адрес выделенного буфера.
// ѕри использовании флага FORMAT_MESSAGE_ALLOCATE_BUFFER система выделит пам€ть и запишет адрес
// в lpMsgBuf.
//
//–азмер буфера(0) Ч не используетс€ при FORMAT_MESSAGE_ALLOCATE_BUFFER, 
// так как система сама определ€ет нужный размер.
//
//јргументы(NULL) Ч не используютс€ при FORMAT_MESSAGE_IGNORE_INSERTS.
