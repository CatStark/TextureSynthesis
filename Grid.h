#pragma once

#ifndef Grid_H
#define Grid_H

#include "Patch.h"

class Grid
{
public:
    Grid(int x , int y); //Constructor
    vector< vector<int> > grid;
    void fill(int);
    void mapGeneration(int);
    vector<int> sortedList; //sorted list of the heightmap

private:
};

#endif