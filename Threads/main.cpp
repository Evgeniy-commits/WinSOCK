#include<iostream>
#include<thread>
#include<mutex>
#include<chrono>
using std::cin;
using std::cout;
using std::endl;
using namespace std::chrono_literals;


bool finish = false;
std::mutex mtx;

void Plus()
{
	while (!finish)
	{
		std::lock_guard<std::mutex> lock(mtx);
		cout << "+ ";
		cout.flush();
		std::this_thread::sleep_for(1ms);
	}
}

void Minus()
{
	while (!finish)
	{
		std::lock_guard<std::mutex> lock(mtx);
		cout << "- ";
		cout.flush();
		std::this_thread::sleep_for(1ms);
	}
}

void main()
{
	setlocale(LC_ALL, "");
	
	std::thread plus_thread(Plus);
	std::thread minus_thread(Minus);

	cin.get();
	finish = true;

	if(plus_thread.joinable())plus_thread.join();
	if(minus_thread.joinable())minus_thread.join();
}
