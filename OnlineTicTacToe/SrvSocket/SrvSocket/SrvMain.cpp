#include <iostream>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 2222

struct fd_set fr;
const int size = 3;
int arrSocket[size];

//////////////////GAME///////////////////////////
const unsigned int row = 3;
const unsigned int col = 3;


static bool CheckSecondRule(char arr[row][col], int c_row, int c_col, int player)
{
	char a = (player) ? 'X' : 'O';
	int index_row = 0;
	int index_col = 0;
	int index_diag_right = 0;

	for (int i = 0; i < row; i++)
	{
		index_row = 0;
		index_col = 0;
		if (arr[i][i] == a)
		{
			index_diag_right++;
			if (index_diag_right >= 3)
			{
				std::cout << "Diagonase Win!" << std::endl;
				return true;
			}
		}
		for (int j = 0; j < col; j++)
		{

			if (arr[j][i] == a)
			{
				index_row++;
				if (index_row >= 3)
				{
					std::cout << "Row win" << std::endl;
					return true;
				}
			}
			if (arr[i][j] == a)
			{
				index_col++;
				if (index_col >= 3)
				{
					std::cout << "Col win" << std::endl;
					return true;
				}
			}
		}
	}

	int index_diag_left = 0;
	int count = 0;

	for (int i = row - 1; i >= 0; i--)
	{
		if (arr[count++][i] == a)
		{
			index_diag_left++;
			if (index_diag_left >= 3)
			{
				return true;
			}
		}
	}
	return false;
}

static bool CheckFirstRule(char arr[row][col], int c_row, int c_col)
{
	if (arr[c_row][c_col] != ' ') return false;

	return true;
}

//////////////////GAME///////////////////////////
char arr[row][col] = {
	{' ', ' ', ' '},
	{' ', ' ', ' '},
	{' ', ' ', ' '}
};

std::string str1, str2;
int player = 1;

//////////////////HandlingMessage///////////////////////////
static void HandlingNewMessage(SOCKET clientSocket, SOCKET servSocket, int num)
{
	char clientBuff[255] = { 0, };
	if (clientSocket == INVALID_SOCKET || recv(clientSocket, clientBuff, sizeof(clientBuff), 0) == SOCKET_ERROR)
	{
		std::cerr << "Something wrong here.... may Client has disconnected! \n";
		WSACleanup();
		closesocket(clientSocket);
		exit(EXIT_FAILURE); // When one socket disconnected, the programm end here
	}
	else
	{
		std::cout << "Client " << num << " :" << clientBuff << std::endl;
		int a[2] = { 0,0 };

		bool bCanPlay = true;

		//std::cout << "Give your answer: ";
		//std::getline(std::cin, str1);

		if (clientBuff == "q")
		{
			WSACleanup();
			closesocket(clientSocket);
			exit(EXIT_FAILURE);
		}

		std::stringstream strstream(clientBuff);
		int count = 0;
		while (std::getline(strstream, str2, ' ') && count < 2)
		{
			a[count++] = std::stoi(str2);
		}

		if (player && bCanPlay)
		{
			if (CheckFirstRule(arr, a[0], a[1]))
			{
				arr[a[0]][a[1]] = 'X';
				bCanPlay = false;
				std::cout << "Player 1 play move successful! \n";
				if (CheckSecondRule(arr, a[0], a[1], player))
				{
					std::cout << "Ok" << "\n";
					std::cout << "Player 1 win! \n";
					WSACleanup();
					closesocket(clientSocket);
					exit(EXIT_FAILURE);
				}
				player = 0;
			}
			else
			{
				std::cout << "Player 1 play failed move! \n";
			}
		}
		if (!player && bCanPlay)
		{
			if (CheckFirstRule(arr, a[0], a[1]))
			{
				arr[a[0]][a[1]] = 'O';
				bCanPlay = false;
				std::cout << "Player 2 play move successful! \n";
				if (CheckSecondRule(arr, a[0], a[1], player))
				{
					std::cout << "Player 2 win! \n";
					WSACleanup();
					closesocket(clientSocket);
					exit(EXIT_FAILURE);
				}
				player = 1;
			}
			else
			{
				std::cout << "Player 2 play failed move! \n";
			}
		}

		/////////////TABLE////////////////
		std::string str = "\n";

		for (int i = 0; i < row; i++)
		{
			str += " -------\n";
			str += std::to_string(i);
			for (int j = 0; j < col; j++)
			{
				str += "|" + std::string(1, arr[i][j]);
			}
			str += "|";
			str += "\n";
		}
		str += " -------\n";
		/////////////////////////////////

		std::cout << str << std::endl;// table for playing

		for (int i = 0; i < size; i++)
		{
			if (arrSocket[i] != 0 && arrSocket[i] != clientSocket && arrSocket[i] != servSocket)
			{
				send(arrSocket[i], str.c_str(), str.length(), 0);
			}
		}
	}
}


static void HandlingNewMessage(SOCKET servSocket)
{
	if (FD_ISSET(servSocket, &fr))
	{
		int addrLen = sizeof(sockaddr);
		SOCKET clientSocket = accept(servSocket, NULL, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			std::cerr << "Client Socket failed with error \n";
			WSACleanup();
			closesocket(clientSocket);
			exit(EXIT_FAILURE);
		}
		else
		{
			for (int i = 0; i < size; i++)
			{
				if (arrSocket[i] == 0)
				{
					arrSocket[i] = clientSocket;
					char servBuff[255] = "Hello, Welcome to my server";
					send(clientSocket, servBuff, sizeof(servBuff), 0);
					break;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < size; i++)
		{
			if (arrSocket != 0 && FD_ISSET(arrSocket[i], &fr))
			{
				HandlingNewMessage(arrSocket[i], servSocket ,i + 1);
			}
		}
	}
}
//////////////////HandlingMessage///////////////////////////

int main()
{
	WSADATA ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) == SOCKET_ERROR)
	{
		std::cerr << "WSA failed with error \n";
		exit(EXIT_FAILURE);
	}

	SOCKET servSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (servSocket == INVALID_SOCKET)
	{
		std::cerr << "Socket failed with error  \n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	servAddr.sin_addr.s_addr = ADDR_ANY;

	// Setting socket non-blocking
	u_long optVal = 1;
	if (ioctlsocket(servSocket, FIONBIO, &optVal) == SOCKET_ERROR)
	{
		std::cerr << "Setting socket failed with error \n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	//Binding Socket with structure
	if (bind(servSocket, (const sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Binding failed with error \n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	if (listen(servSocket, 3) == SOCKET_ERROR)
	{
		std::cerr << "Listening failed with error \n";
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	while (true)
	{
		FD_ZERO(&fr);
		FD_SET(servSocket, &fr);

		for (int i = 0; i < size; i++)
		{
			if (arrSocket[i] != 0)
			{
				FD_SET(arrSocket[i], &fr);
			}
		}

		int maxNfds = servSocket + 1;

		int checkSelect = select(maxNfds, &fr, NULL, NULL, &tv);
		if (checkSelect > 0)
		{
			HandlingNewMessage(servSocket);
		}
		else if (checkSelect == 0)
		{
			//Waiting for Client Request
		}
		else
		{
			std::cerr << "Select failed with error \n";
			WSACleanup();
			exit(EXIT_FAILURE);
		}
	}


	closesocket(servSocket);

	return 0;
}