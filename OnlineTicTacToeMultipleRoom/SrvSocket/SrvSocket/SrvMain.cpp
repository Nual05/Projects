#include <iostream>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include "GameManager.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT 2222
#define SIZE_GROUP 2
#define SIZE_PLAYER 2

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
/////////////////////////////////////////////////

struct fd_set fr;
GameManager gm[SIZE_GROUP];

//////////////////HandlingMessage///////////////////////////
static void HandlingNewMessage(SOCKET clientSocket, SOCKET servSocket, int RoomNumber)
{
	char clientBuff[255] = { 0, };
	if (clientSocket == INVALID_SOCKET || recv(clientSocket, clientBuff, sizeof(clientBuff), 0) == SOCKET_ERROR)
	{
		std::cerr << "Something wrong here.... may Client has disconnected! \n";
		SOCKET newSocket = gm[RoomNumber].RemovePlayer(clientSocket);
		if (newSocket != INVALID_SOCKET)
		{
			char buff[255] = "You need to wait for someone join... \n";
			send(newSocket, buff, sizeof(buff), 0);
			memset(arr, ' ', sizeof(arr));
		}

		closesocket(clientSocket);
	}
	else
	{
		std::cout << "Client: " << clientBuff << std::endl;

		int a[2] = { 0,0 };

		bool bCanPlay = true;
		std::string word  = " ";

		std::stringstream strstream(clientBuff);
		std::stringstream checkString(clientBuff);

		checkString >> word;
		
		if(word == "0" || word == "1" || word == "2")
		{

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
					//std::cout << "Player 1 play move successful! \n";
					if (CheckSecondRule(arr, a[0], a[1], player))
					{
						char buff[255] =  "Player 1 (X) win! \n";
						for (int i = 0; i < SIZE_PLAYER; i++)
						{
							if (gm[RoomNumber].playerList[i] != 0)
							{
								send(gm[RoomNumber].playerList[i], buff, sizeof(buff), 0);
							}
						}
						WSACleanup();
						closesocket(clientSocket);
						exit(EXIT_FAILURE);
					}
					player = 0;
				}
				else
				{
					//std::cout << "Player 1 play failed move! \n";
				}
			}
			if (!player && bCanPlay)
			{
				if (CheckFirstRule(arr, a[0], a[1]))
				{
					arr[a[0]][a[1]] = 'O';
					bCanPlay = false;
					//std::cout << "Player 2 play move successful! \n";
					if (CheckSecondRule(arr, a[0], a[1], player))
					{
						char buff[255] = "Player 2 (O) win! \n";
						for (int i = 0; i < SIZE_PLAYER; i++)
						{
							if (gm[RoomNumber].playerList[i] != 0)
							{
								send(gm[RoomNumber].playerList[i], buff, sizeof(buff), 0);
							}
						}
						WSACleanup();
						closesocket(clientSocket);
						exit(EXIT_FAILURE);
					}
					player = 1;
				}
				else
				{
					//std::cout << "Player 2 play failed move! \n";
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


			for (int i = 0; i < SIZE_PLAYER; i++)
			{
				if (gm[RoomNumber].playerList[i] != 0)
				{
					send(gm[RoomNumber].playerList[i], str.c_str(), str.length(), 0);
				}
			}

		}
		else 
		{
			for (int i = 0; i < SIZE_PLAYER; i++)
			{
				if (gm[RoomNumber].playerList[i] != 0)
				{
					send(gm[RoomNumber].playerList[i], clientBuff, sizeof(clientBuff), 0);
				}
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
			int i;
			for (i = 0; i < SIZE_GROUP; i++)
			{
				if (!gm[i].CheckFullRoom())
				{
					gm[i].AddPlayer(clientSocket);
					char servBuff[255] = "Hello, Welcome to my server, waiting until someone join to play \n";
					send(clientSocket, servBuff, sizeof(servBuff), 0);
					break;
				}
			}
			if (i == (SIZE_GROUP-1))
			{
				std::cerr << "The last room has been created! \n";
			}
			//
		}
	}
	else
	{
		
		for (int i = 0; i < SIZE_GROUP; i++)
		{
			for (int j = 0; j < SIZE_PLAYER; j++)
			{
				if (gm[i].playerList[j] != 0 && FD_ISSET(gm[i].playerList[j], &fr))
				{
					HandlingNewMessage(gm[i].playerList[j], servSocket, i );
				}
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

		for (int i = 0; i < SIZE_GROUP; i++)
		{
			for (int j = 0; j < SIZE_PLAYER; j++)
			{
				if (gm[i].playerList[j] != 0)
				{
					FD_SET(gm[i].playerList[j], &fr);
				}
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