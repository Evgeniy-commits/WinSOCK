#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<chrono>
using std::cin;
using std::cout;
using std::endl;
using namespace std::chrono_literals;


bool finish = false;
std::mutex mtx;
std::condition_variable cv;
bool plus_turn = true;

void Plus()
{
	while (!finish)
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [] {return plus_turn || finish; });
		if (finish) break;

		cout << "+ ";
		cout.flush();

		plus_turn = false;
		cv.notify_one();
	}
}

void Minus()
{
	while (!finish)
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [] {return !plus_turn || finish; });

		cout << "- ";
		cout.flush();

		plus_turn = true;
		cv.notify_one();
	}
}

void main()
{
	setlocale(LC_ALL, "");
	
	std::thread plus_thread(Plus);
	std::thread minus_thread(Minus);

	cin.get();
	{
		std::lock_guard<std::mutex> lock(mtx);
		finish = true;
	}

	cv.notify_all();

	if(plus_thread.joinable())plus_thread.join();
	if(minus_thread.joinable())minus_thread.join();
}
