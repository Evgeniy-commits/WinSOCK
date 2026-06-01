#include "FormatLastError.h"

LPSTR FormatLastError(DWORD dwError, CHAR szBuffer[])
{
	LPSTR lpBuf = nullptr;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpBuf,
		0,
		NULL

	);
	sprintf(szBuffer, "(%i) : %s", dwError, lpBuf);
	LocalFree(lpBuf);
	return szBuffer;
}

VOID clientInfo(SOCKET client_socket, sockaddr_in& clientAddr, INT szClientAddr, ofstream& logS)
{
	//Получаем IP адрес клиента
	CHAR clientIp[INET_ADDRSTRLEN] = {};
	inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, INET_ADDRSTRLEN);

	//Получаем порт клиента
	int clientPort = ntohs(clientAddr.sin_port);

	//Получаем текущее время
	chrono::system_clock::time_point now = chrono::system_clock::now();
	time_t curTime = chrono::system_clock::to_time_t(now);
	char timeBuf[100];
	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", localtime(&curTime));

	//Вывод Info
	cout << "Client connected..." << endl;
	cout << "IP: " << clientIp << endl;
	cout << "Port: " << clientPort << endl;
	cout << "Time: " << timeBuf << endl;

	logS << "Connected: IP = " << clientIp << ", Port = " << clientPort << ",Time = " << timeBuf << endl;
}