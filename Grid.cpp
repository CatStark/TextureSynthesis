#include "Grid.h"

Grid::Grid(int x, int y) //Constructor
{
	//grid.resize(x, vector<int>(y));
	grid.resize(x);
	for (int i = 0; i < x; i++)	{
		grid[i].resize(y);
	}
}

//Fill the grid with random values
void Grid::fill(int backgroundPercentage)
{
	#pragma omp for 
	for (int j = 0; j < grid[1].size() ; j++){
		for (int i = 0; i < grid.size(); i++){
			double start = rand() % 255;
			grid[i][j] = start;
			sortedList.push_back(start);
			stable_sort(sortedList.begin(), sortedList.end()); //sort the list
			cout << "[" <</*<< i << "][" << j << "]:" << */grid[i][j] << "]";
		}
		cout << endl;
	}

	//apply noise into the matrix
	mapGeneration(backgroundPercentage);
}

//For each cell we replace its value with the average value of all the cells around it
void Grid::mapGeneration(int backgroundPercentage)
{
	//we run through each cell again and if it's less than the cell average the cell is type A or type B
	//0 is background 1 is details
	int flag = sortedList.size()/(backgroundPercentage/10);
	flag = sortedList[flag];
	#pragma omp for 
	for (int j = 0; j < grid[1].size() ; j++){
		for (int i = 0; i < grid.size(); i++){
			if (grid[i][j] > flag)
				grid[i][j] = 0;
			else if (grid[i][j] <= flag && grid[i][j] >= flag/2 )
				grid[i][j] = 1;
			else
				grid[i][j] = 2;
			cout << "[" << grid[i][j] << "]";
		}
		cout << endl;
	}

}