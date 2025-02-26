#include "GameEngine.h"
#include <stdlib.h>
#include <iostream>
using namespace std;
GameEngine::GameEngine(const int _cols, const int _rows) 
{
	cols = _cols;
	rows = _rows;
	field = new bool* [*COLS];
	for (int i = 0; i < *COLS; i++) field[i] = new bool[*ROWS];
	tempField = new bool* [*COLS];
	for (int i = 0; i < *COLS; i++) tempField[i] = new bool[*ROWS];
 }
GameEngine::~GameEngine() 
{
	for (int i = 0; i < cols; i++) delete[] field[i];
	delete[] field;
	for (int i = 0; i < cols; i++) delete[] tempField[i];
	delete[] tempField;
}
void GameEngine::StartGame() 
{
	for (int x = 0; x < cols; x++) 
	{
		for (int y = 0; y < rows; y++) 
		{
			int a = rand();
			if (a % 8 == 0) field[x][y] = true;
			else field[x][y] = false;
		}
	}
}
void GameEngine::NextIteration() 
{
	for (int x = 0; x < cols; x++)
	{
		for (int y = 0; y < rows; y++)
		{
			tempField[x][y] = field[x][y];
		}
	}
	for (int x = 0; x < cols; x++)
	{
		for (int y = 0; y < rows; y++) 
		{
			int value = CountNeighbors(x, y);
			if (!field[x][y])
			{
				tempField[x][y] = Be(value);
			}
			else if(field[x][y])
			{
				tempField[x][y] = Still(value);
			}
		}
	}
	for (int x = 0; x < cols; x++)
	{
		for (int y = 0; y < rows; y++)
		{
			field[x][y] = tempField[x][y];
		}
	}
}
bool GameEngine::Be(int value)
{
	if (value == 3) return true;
	else return false;
}
bool GameEngine::Still(int value)
{
	if (value == 2 || value == 3) return true;
	else return false;
}
int GameEngine::CountNeighbors(int x, int y) 
{
	int res = 0;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			if (i == 0 && j == 0) continue;
			if (field[(x + i + cols) % cols][(y + j + rows) % rows]) res++;
		}
	}
	return res;
}