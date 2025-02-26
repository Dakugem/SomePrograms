#pragma once
class GameEngine
{
private:
	int cols, rows;
	int *COLS = &cols, *ROWS = &rows;
	int CountNeighbors(int, int);
	bool Be(int);
	bool Still(int);
public:
	bool** field;
	bool** tempField;
	GameEngine(const int, const int);
	~GameEngine();
	void StartGame();
	void NextIteration();
};

