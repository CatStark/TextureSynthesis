#include "Patch.h"
#include <iostream>

//Constructors
Patch::Patch(Mat &img)
{
	//std::cout << "---------- Patch created ----------" << endl;

	//The patch has to be a square, so check what is bigger (width or height) and make a square
    if (img.cols >= img.rows){
        width = img.cols/4;
        height = width;
    }
    else {
        height = img.rows /4;
        width = height; 
    }
    typeOfTexture = -1;
}

Patch::Patch()
{
	//std::cout << "---------- Patch created ----------" << endl;
}


