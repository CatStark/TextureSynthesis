#include "FinalImage.h"

#include <iostream>
#include <math.h>
#include <algorithm>
#include <sys/time.h>


#define ATD at<double>
#define elif else if

int main( int argc, char** argv ){

    int option = 0;

    struct timeval start, end;
    double delta;
    

    srand(time(NULL)); //Seed to get randmon patches
    
    // Create empty output image.
    Mat InputImg; //Background
    Mat InputImg2; //Details
    Mat InputImg3;
    Mat result;
    std::vector<Mat> inputSamples;
    
    //Load input images
    //img = imread("Moon.jpg");
    InputImg = imread("Textures/AST3.jpg");
    InputImg2 = imread("Textures/AST2.jpg");
    InputImg3 = imread("Textures/AST1.jpg");

    inputSamples.push_back(InputImg);
    inputSamples.push_back(InputImg2);
    inputSamples.push_back(InputImg3);

    int backgroundPorcentage, detailsPorcentage = 0; 
    std::vector<int> lightVector; //From where is the light coming 1) Left 2)Up 3)Right 4)Down
    int lightDirection, mainLight; //mainLight is the one from the image with the biggest %

    //Create first patch from img
    Patch _patch(InputImg); 

    //Create target
    Patch _target(InputImg);

    //Create empty output texure
    FinalImage _finalImage(InputImg, 256, 256, 10);
   
    //Chose mode
    cout << "1. Random    2. Image Quilting " << endl;
    cin >> option;
    
    if (option == 1)
        result =  _finalImage.placeRandomly(_patch, InputImg);
    else if (option == 2)
    {
        cout << "How much porcentage you want to give to the Background (0 - 100) " << endl;
        //cin >> backgroundPorcentage;
        backgroundPorcentage = 50; //hardcoded just for debugging 
        cout << "How much porcentage you want to give to the details ( 0 - 100) " << endl;
        //cin >> detailsPorcentage;
        detailsPorcentage = 50;
        //TODO verification that background and details sums to 100%
        
        for (int i = 0; i< inputSamples.size(); i++)
        {
            cout << "From where is the light coming for the " << i << " input" << endl;
            cout << "1) Left 2)Up 3)Right 4)Down" << endl;
            cin >> lightDirection;
            if (i == 0)
                mainLight = lightDirection;
            else
            {
                if (lightDirection != mainLight)
                {
                    transpose(inputSamples.at(i), inputSamples.at(i));
                    if (sqrt((lightDirection + mainLight)) == 0 ) 
                        flip(inputSamples.at(i), inputSamples.at(i), 1);
                    else
                        flip(inputSamples.at(i), inputSamples.at(i), 2);
                }
            }
        }
        gettimeofday(&start, NULL);
        result = _finalImage.textureSynthesis(_patch, _target, InputImg, InputImg2, InputImg3, backgroundPorcentage, detailsPorcentage);
    }
   /* else if (option == 3)
        result = img;*/
    result.convertTo(result, CV_8UC1);
    imwrite("final.jpg", result);
    imshow("Final", result);
    //imshow("img", img);


    gettimeofday(&end, NULL);

    delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    cout<<"used time: "<< delta  << " second"<<endl;
    
    waitKey(0);
    return 0;
}