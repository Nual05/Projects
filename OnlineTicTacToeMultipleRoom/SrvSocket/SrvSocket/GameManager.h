#pragma once
#include <iostream>
#include <WinSock2.h>


class GameManager
{
private:
	bool IsFull = false;
public:
	//Variables
	SOCKET playerList[2];

	//Constructors
	GameManager();
	GameManager(int size);

	//Functions
	void AddPlayer(const SOCKET &pSock);
	SOCKET RemovePlayer(const SOCKET &pSock);
	bool CheckFullRoom();
};
