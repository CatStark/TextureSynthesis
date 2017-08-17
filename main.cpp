/* Susana M. Guzmán
Master Thesis 
Universität Bremen
*/


#include "FinalImage.h"

#include <iostream>
#include <math.h>
#include <algorithm>
#include <sys/time.h>


#define ATD at<double>
#define elif else if

Mat src,img,ROI;
Rect cropRect(0,0,0,0);
 Point P1(0,0);
 Point P2(0,0);

const char* winName="Crop Image";
bool clicked=false;
int i=0;
char imgName[15];
std::vector<string> input_Name;
string temp;


void checkBoundary(){
       //check croping rectangle exceed image boundary
       if(cropRect.width>img.cols-cropRect.x)
         cropRect.width=img.cols-cropRect.x;

       if(cropRect.height>img.rows-cropRect.y)
         cropRect.height=img.rows-cropRect.y;

        if(cropRect.x<0)
         cropRect.x=0;

       if(cropRect.y<0)
         cropRect.height=0;
}

Mat showImage(){
    img=src.clone();
    checkBoundary();
    if(cropRect.width>0&&cropRect.height>0){
        ROI=src(cropRect);
        imshow("cropped",ROI);
    }
    rectangle(img, cropRect, Scalar(0,255,0), 1, 8, 0 );
    imshow(winName,img);
    return ROI;
}

void onMouse( int event, int x, int y, int f, void* )
{


    switch(event){

        case  CV_EVENT_LBUTTONDOWN  :
                                        clicked=true;

                                        P1.x=x;
                                        P1.y=y;
                                        P2.x=x;
                                        P2.y=y;
                                        break;

        case  CV_EVENT_LBUTTONUP    :
                                        P2.x=x;
                                        P2.y=y;
                                        clicked=false;
                                        break;

        case  CV_EVENT_MOUSEMOVE    :
                                        if(clicked){
                                        P2.x=x;
                                        P2.y=y;
                                        }
                                        break;

        default                     :   break;


    }


    if(clicked){
     if(P1.x>P2.x){ cropRect.x=P2.x;
                       cropRect.width=P1.x-P2.x; }
        else {         cropRect.x=P1.x;
                       cropRect.width=P2.x-P1.x; }

        if(P1.y>P2.y){ cropRect.y=P2.y;
                       cropRect.height=P1.y-P2.y; }
        else {         cropRect.y=P1.y;
                       cropRect.height=P2.y-P1.y; }

    }


showImage();
}

int main( int argc, char** argv )
{

    int option, number_of_Inputs = 0;
    int mode = 0; //0 = no user interaction for feauture perservation, 1 = user interaction for feauture perservation 
    struct timeval start, end;
    double delta;
    srand(time(NULL)); //Seed to get randmon patches
    
    // Create empty output image.
    Mat InputImg; //Background
    Mat InputImg2; //Details
    Mat InputImg3;
    Mat result;
    std::vector<Mat> inputSamples;
    std::vector<Mat> InputImg_;

    //Background
    InputImg = imread("Textures/AST3.jpg");
    //Details
        InputImg2 = imread("Textures/AST2.jpg");
        InputImg3 = imread("Textures/AST1.jpg");


    //Feauture perservation
    char response;
    cout<<"Do you want to specify the features manually? Y/n"<<endl<<endl;
    cin >> response;
    
    if (response == 'y')
    {
        mode = 1;
        cout<<"Click and drag for Selection"<<endl<<endl;
        cout<<"------> Press 's' to save"<<endl<<endl;
        cout<<"------> Press 'r' to reset"<<endl;
        cout<<"------> Press 'Esc' to quit"<<endl<<endl;
    

        for (int i = 0; i < 2; i++)
        {
            if ( i == 0)
                src=InputImg2;
            else
                src=InputImg3;
            namedWindow(winName,WINDOW_NORMAL);
            setMouseCallback(winName,onMouse,NULL );
            imshow(winName, src);    
            while(1){
              char c=waitKey();
              if(c=='s'&&ROI.data)
              {
                sprintf(imgName,"%d.jpg",i++);
                imwrite(imgName,ROI);
                cout<<"  Saved "<<imgName<<endl;
              }
              if(c==27) break;
              if(c=='r') {cropRect.x=0;cropRect.y=0;cropRect.width=0;cropRect.height=0;}
              ROI = showImage();
              //imshow("cropped",ROI);
            }
            destroyAllWindows();

            if (i == 0)
                InputImg2 = ROI;
            else 
                InputImg3 = ROI;
        }
    }
    
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
        result = _finalImage.textureSynthesis(_patch, _target, InputImg, InputImg2, InputImg3, backgroundPorcentage, detailsPorcentage,  mode);
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
