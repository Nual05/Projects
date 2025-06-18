#include <iostream>
#include <sstream>
#include <string>

const unsigned int row = 3;
const unsigned int col = 3;


static bool CheckSecondRule(char arr[row][col], int c_row, int c_col, int player )
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

int main()
{

	char arr[row][col] = {
		{' ', ' ', ' '},
		{' ', ' ', ' '},
		{' ', ' ', ' '}
	};

	

	std::string str1, str2;
	int player = 1;
	

	do
	{
		int a[2] = { 0,0 };
		/////////////TABLE////////////////
		std::string str = "";

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

		bool bCanPlay = true;
		
		std::cout << str << std::endl;// table for playing

		std::cout << "Give your answer: ";
		std::getline(std::cin, str1);

		if (str1 == "q") return 1;

		std::stringstream strstream(str1);
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
					return 0;
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
					return 0;
				}
				player = 1;
			}
			else
			{
				std::cout << "Player 2 play failed move! \n";
			}
		}
		
	} while (str1 != "q");


	return 0;
}