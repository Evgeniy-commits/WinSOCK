#include<iostream>
#include<thread>
#include<windows.h>
#include<chrono>
using std::cin;
using std::cout;
using std::endl;
using namespace std::chrono_literals;


bool finish = false;
HANDLE hMtx;

void Plus()
{
	while (!finish)
	{
		WaitForSingleObject(hMtx, INFINITE);
		cout << "+ ";
		cout.flush();
		//std::this_thread::sleep_for(1ms);
		ReleaseMutex(hMtx);
	}
}

void Minus()
{
	while (!finish)
	{
		WaitForSingleObject(hMtx, INFINITE);
		cout << "- ";
		cout.flush();
		//std::this_thread::sleep_for(1ms);
		ReleaseMutex(hMtx);
	}
}

void main()
{
	setlocale(LC_ALL, "");
	
	hMtx = CreateMutex(NULL, FALSE, NULL);

	std::thread plus_thread(Plus);
	std::thread minus_thread(Minus);

	cin.get();
	finish = true;

	if(plus_thread.joinable())plus_thread.join();
	if(minus_thread.joinable())minus_thread.join();
}
