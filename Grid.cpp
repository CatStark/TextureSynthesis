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
void Grid::fill(int backgroundPercentage, int detailsPercentage)
{
	#pragma omp for 
	for (int j = 0; j < grid[1].size() ; j++){
		for (int i = 0; i < grid.size(); i++){
			double start = rand() % 255;
			grid[i][j] = start;
			//sortedList.push_back(start);
			//stable_sort(sortedList.begin(), sortedList.end()); //sort the list
			cout << "[" <</*<< i << "][" << j << "]:" << */grid[i][j] << "]";
		}
		cout << endl;
	}

	//apply noise into the matrix
	getAverageOfGrid();
	mapGeneration(backgroundPercentage, detailsPercentage);
}

void Grid::getAverageOfGrid()// For each cell we replace its value with the average value of all the cells around it
{
	int tmpValue, tempAvg = 0;
	int countOfValues; //How many data we have per cell to make the average
	double totalAvg = 0;
	for (int j = 0; j < grid[1].size() ; j++){
		for (int i = 0; i < grid.size(); i++){
			countOfValues = 0;
			tmpValue = 0;
			//cout << "current i: " << i << " j: " << j << "   " << grid[i][j] << endl;
			if(i-1 >= 0){ //Check left side
				tmpValue += grid[i-1][j];
				countOfValues++;
				if(j+1 < grid[1].size()){ //Down
					tmpValue += grid[i-1][j+1];
					countOfValues++;
				}
				if (j-1 < grid[1].size()){ //Up
					tmpValue += grid[i-1][j-1];
					countOfValues++;
				}
			}

			if(i+1 < grid.size()){ //Check right side
				tmpValue += grid[i+1][j];
				countOfValues++;
				if(j+1 < grid[1].size()){ //Down
					tmpValue += grid[i+1][j+1];
					countOfValues++;
				}
				if (j-1 < grid[1].size()){ //Up
					tmpValue += grid[i+1][j-1];
					countOfValues++;
				}
			}

			if(j-1 >= 0){
				tmpValue += grid[i][j-1];
				countOfValues++;
			}
			
			if(j+1 < grid[1].size()){
				tmpValue += grid[i][j+1];
				countOfValues++;
			}
			//cout << endl;

			totalAvg += tmpValue/countOfValues;
			grid[i][j] = tmpValue/countOfValues;
			sortedList.push_back(grid[i][j]);
			cout << "[" << grid[i][j] << "]";
		}
		totalAvg = totalAvg/grid.size();
		tempAvg += totalAvg;
		cout << endl;
	}
	totalAvg = tempAvg/grid[1].size();
	stable_sort(sortedList.begin(), sortedList.end()); //sort the list
}

//For each cell we replace its value with the average value of all the cells around it
void Grid::mapGeneration(int backgroundPercentage, int detailsPercentage)
{
	//we run through each cell again and if it's less than the cell average the cell is type A or type B
	//double flag = sortedList.size() * (backgroundPercentage/100);
	double flag = sortedList.size() * .90;
	flag = sortedList[flag];
	for (int j = 0; j < grid[1].size() ; j++){
		for (int i = 0; i < grid.size(); i++){
			if (grid[i][j] < flag)
				grid[i][j] = 0;
			else if (grid[i][j] >= flag && grid[i][j] <= flag + (sortedList.size() * .10))
				grid[i][j] = 1;
			else
				grid[i][j] = 2;
			cout << "[" << grid[i][j] << "]";
		}
		cout << endl;
	}

}