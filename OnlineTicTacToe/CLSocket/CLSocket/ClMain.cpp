#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 2222
#define SERVER_IP "127.0.0.1"

static void RecevingMessage(SOCKET clientSocket)
{
	char servBuff[255] = { 0, };
	while (true)
	{
		if (recv(clientSocket, servBuff, sizeof(servBuff), 0) != SOCKET_ERROR)
		{
			std::cout << "Server: " << servBuff << std::endl;
		}
	}
}

static void SendingMessage(SOCKET clientSocket)
{
	while (true)
	{
		std::string str = " ";
		std::getline(std::cin, str);
		send(clientSocket, str.c_str(), str.length(), 0);
	}
}

int main()
{
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) == SOCKET_ERROR)
	{
		std::cerr << "WSA failed with error \n";
		exit(EXIT_FAILURE);
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cerr << "Socket failed with error \n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_IP, &servAddr.sin_addr);

	if (connect(clientSocket, (const sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Connecting failed with error \n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
	{
		char servBuff[255] = { 0, };
		recv(clientSocket, servBuff, sizeof(servBuff), 0);
		std::cout << "Server: " << servBuff << std::endl;

		std::thread recvThread(RecevingMessage, clientSocket);
		recvThread.detach();

		std::thread sendThread(SendingMessage, clientSocket);
		sendThread.detach();

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}


	closesocket(clientSocket);



	return 0;
}