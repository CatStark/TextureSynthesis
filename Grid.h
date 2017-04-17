#pragma once

#ifndef Grid_H
#define Grid_H

#include "Patch.h"

class Grid
{
public:
    Grid(int x , int y); //Constructor
    vector< vector<int> > grid;
    void fill(int backgroundPercentage, int detailsPercentage);
    void getAverageOfGrid();
    void mapGeneration(int backgroundPercentage, int detailsPercentage);
    vector<int> sortedList; //sorted list of the heightmap

private:
};

#endif