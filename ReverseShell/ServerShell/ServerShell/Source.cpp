#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <cstring>


#pragma comment(lib, "Ws2_32.lib")

#define PORT 8888

int main()
{
	
	
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) == SOCKET_ERROR)
	{
		std::cerr << "WSA failed with error" << std::endl;
		return 1;
	}

	SOCKET servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (servSock == INVALID_SOCKET)
	{
		std::cerr << "Socket failed with error" << std::endl;
		WSACleanup();
		return 1;
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(servSock, (const sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Binding failed" << std::endl;
		closesocket(servSock);
		WSACleanup();
		return 1;
	}

	if (listen(servSock, 5) == SOCKET_ERROR)
	{
		std::cerr << "Listening failed" << std::endl;
		closesocket(servSock);
		WSACleanup();
		return 1;
	}

	while (true)
	{
		int sockLen = sizeof(struct sockaddr);
		SOCKET client = accept(servSock, NULL, &sockLen);
		
		if (client == INVALID_SOCKET)
		{
			std::cerr << "Client Socket failed with error \n";
			WSACleanup();
			closesocket(client);
			exit(EXIT_FAILURE);
			break;
		}
		else
		{
			while (true)
			{
				std::cout << "Write input: ";
				std::string str = " ";
				std::getline(std::cin, str);

				if (send(client, str.c_str(), str.size(), 0) != SOCKET_ERROR)
				{
					char buffer[5000] = { 0 };
					if (recv(client, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
					{
						std::cout << buffer << std::endl;
					}
				}

			}
		}

	}


	closesocket(servSock);
	WSACleanup();
	
	return 0;
}