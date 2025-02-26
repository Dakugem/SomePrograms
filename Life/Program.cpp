#include <iostream>
#include <conio.h> 
#include <windows.h>
#include <time.h>
#include "GameEngine.h"
using namespace std;

const int
cols = 630,
rows = 168;
GameEngine gameEngine(cols, rows);

void drawField(HANDLE wHnd);

int main() {
	HANDLE wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
	SMALL_RECT windowSize = { 0, 0, cols - 1, rows - 1 };
	SetConsoleWindowInfo(wHnd, TRUE, &windowSize);
	COORD bufferSize = { cols, rows };
	SetConsoleScreenBufferSize(wHnd, bufferSize);
	SetConsoleTextAttribute(wHnd, FOREGROUND_RED | FOREGROUND_INTENSITY);

	gameEngine.StartGame();
	while (true) {
		drawField(wHnd);
		gameEngine.NextIteration();
	}
}

void drawField(HANDLE wHnd) {
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(wHnd, &csbiInfo);
	CHAR_INFO *picBuffer = new CHAR_INFO[cols * rows];
	for (int i = 0; i < cols * rows; i++) {
		picBuffer[i].Char.AsciiChar = ' ';
		picBuffer[i].Attributes = csbiInfo.wAttributes;
	}

	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			if (gameEngine.field[x][y]) picBuffer[x + y * cols].Char.AsciiChar = '#';
		}
	}

	SMALL_RECT writeArea = { 0, 0, cols - 1, rows - 1 };
	WriteConsoleOutputA(wHnd, picBuffer, { cols, rows }, {0, 0}, &writeArea);
}