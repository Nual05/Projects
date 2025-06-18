#include "GameManager.h"

GameManager::GameManager()
{

}

GameManager::GameManager(int size)
{
	playerList[size];
}

void GameManager::AddPlayer(const SOCKET &pSock)
{
	int i;
	for (i = 0; i < 2; i++)
	{
		if (playerList[i] == 0)
		{
			playerList[i] = pSock;
			break;
		}
	}
	if (i >= 1)
	{
		IsFull = true;
		char buff[255] = "Some one join... try to say Hello, start game with 0 or 1 ";
		for (int j = 0; j < 2; j++)
		{
			send(playerList[j], buff, sizeof(buff), 0);
		}
		
	}
	
}

SOCKET GameManager::RemovePlayer(const SOCKET& pSock)
{
	for (int i = 0; i < 2; i++)
	{
		if (playerList[i] == pSock)
		{
			playerList[i] = 0;
			IsFull = false;
			break;
		}
	}
	for (int i = 0; i < 2; i++)
	{
		if (playerList[i] != 0)
		{
			return playerList[i];
		}
	}
}

bool GameManager::CheckFullRoom()
{
	return IsFull;
}