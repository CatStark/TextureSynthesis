#include "FinalImage.h"


typedef Graph<int,int,int> GraphType;

struct findRepeatedPatch
{
    double error;
    findRepeatedPatch(double error) : error(error) {}
    bool operator() (const Patch& m) const
    {
        return m.error == error;
    }
};

//Constructor
FinalImage::FinalImage(Mat &img, int y_expand, int x_expand, int windowSize)
{
    newimg = Mat::zeros(img.rows + y_expand  , img.cols + x_expand, CV_64FC3);
    newimg.convertTo(newimg, CV_8UC1);
    width = newimg.cols;
    height = newimg.rows;
    backgroundPorcentageTmp = 0;
    cout << " ------------Output image created--------------" << endl;
}


void circleDetection(Mat& src)
{
    cv::Mat gray, canny;
    /// Convert it to gray
    cv::cvtColor( src, gray, CV_BGR2GRAY );

    // compute canny (don't blur with that image quality!!)
    cv::Canny(gray, canny, 200,20);
    //cv::namedWindow("canny2"); cv::imshow("canny2", canny>0);


    std::vector<cv::Vec3f> circles;

    /// Apply the Hough Transform to find the circles
    cv::HoughCircles( gray, circles, CV_HOUGH_GRADIENT, 1, src.rows/8, 100, 20, src.rows/5006, 0 );

    /// Draw the circles detected
    for( size_t i = 0; i < circles.size(); i++ ) 
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        cv::circle( src, center, 3, Scalar(0,255,255), -1);
        cv::circle( src, center, radius, Scalar(0,0,255), 1 );
    }

    //compute distance transform:
    cv::Mat dt;
    cv::distanceTransform(255-(canny>0), dt, CV_DIST_L2 ,3);
    cv::namedWindow("distance transform"); cv::imshow("distance transform", dt/255.0f);
    cout << " there are " << circles.size() << " circles in the image " << endl;

    // test for semi-circles:
    float minInlierDist = 2.0f;
    for( size_t i = 0; i < circles.size(); i++ ) 
    {
        // test inlier percentage:
        // sample the circle and check for distance to the next edge
        unsigned int counter = 0;
        unsigned int inlier = 0;

        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        // maximal distance of inlier might depend on the size of the circle
        float maxInlierDist = radius/55.0f;
        if(maxInlierDist < minInlierDist) maxInlierDist = minInlierDist;

        //TODO: maybe paramter incrementation might depend on circle size!
        for(float t =0; t<2*3.14159265359f; t+= 0.1f) 
        {
            counter++;
            float cX = radius*cos(t) + circles[i][0];
            float cY = radius*sin(t) + circles[i][1];

            if(dt.at<float>(cY,cX) < maxInlierDist) 
            {
                inlier++;
                cv::circle(src, cv::Point2i(cX,cY),3, cv::Scalar(0,255,0));
            } 
           else
                cv::circle(src, cv::Point2i(cX,cY),3, cv::Scalar(255,0,0));
        }
        std::cout << 100.0f*(float)inlier/(float)counter << " % of a circle with radius " << radius << " detected" << std::endl;
    }

    cv::namedWindow("output"); cv::imshow("output", src);
    cv::imwrite("houghLinesComputed.png", src);

}

Mat FinalImage::graph_Cut(Mat& A, Mat& B, int overlap, int orientation)
{
    assert(A.data);
    assert(B.data);
    if (orientation == 1)
        assert(A.rows == B.rows);
    else if (orientation == 2)
        assert(A.cols == B.cols);

    Mat graphcut;
    Mat graphcut_and_cutline;

    int xoffset = 0;
    int yoffset = 0;
    int _rows, _cols; //Of Mat

    if (orientation == 1)
    {
        _rows = A.rows;
        _cols = A.cols + B.cols - overlap;
        xoffset = A.cols - overlap;
    }
    else if ( orientation == 2)
    {
        _rows = A.rows + B.rows - overlap;
        _cols = A.cols;
        yoffset = A.rows - overlap;
    }

    Mat no_graphcut(_rows, _cols, A.type() );
    A.copyTo(no_graphcut(Rect(0, 0, A.cols, A.rows)));
    B.copyTo(no_graphcut(Rect(xoffset, yoffset, B.cols, B.rows)));

    int est_nodes;
    if (orientation == 1)      
        est_nodes = A.rows * overlap;
    else  
        est_nodes = A.cols * overlap;
    int est_edges = est_nodes * 4;

    GraphType g(est_nodes, est_edges);

    for(int i=0; i < est_nodes; i++) {
        g.add_node();
    }

    if (orientation == 1)
    {

        // Set the source/sink weights
        #pragma omp for 
        for(int y=0; y < A.rows; y++) {
            g.add_tweights(y*overlap + 0, INT_MAX, 0);
            g.add_tweights(y*overlap + overlap-1, 0, INT_MAX);
        }

        // Set edge weights
        #pragma omp parallel for
        for(int y=0; y < A.rows; y++) { //Change this 
            for(int x=0; x < overlap; x++) {
                int idx = y*overlap + x;

                Vec3b a0 = A.at<Vec3b>(y, xoffset + x);
                Vec3b b0 = B.at<Vec3b>(y, x);
                double cap0 = norm(a0, b0);

                // Add right edge
                if(x+1 < overlap) {
                    Vec3b a1 = A.at<Vec3b>(y, xoffset + x + 1);
                    Vec3b b1 = B.at<Vec3b>(y, x + 1);

                    double cap1 = norm(a1, b1);
                    #pragma omp critical
                    g.add_edge(idx, idx + 1, (int)(cap0 + cap1), (int)(cap0 + cap1));
                }

                // Add bottom edge
                if(y+1 < A.rows) {
                    Vec3b a2 = A.at<Vec3b>(y+1, xoffset + x);
                    Vec3b b2 = B.at<Vec3b>(y+1, x);

                    double cap2 = norm(a2, b2);
                    #pragma omp critical
                    g.add_edge(idx, idx + overlap, (int)(cap0 + cap2), (int)(cap0 + cap2));
                }
            }
        }
    }
    else 
    {
        //Set the source/sink weights
        for(int x=0; x < A.cols; x++) {
            g.add_tweights(x*overlap + 0, INT_MAX, 0); // Add the Terminal nodes 
            g.add_tweights(x*overlap + overlap - 1, 0, INT_MAX);
        }

        #pragma omp parallel for collapse(2)
        for(int x=0; x < A.cols; x++) {
            for( int y=0; y < overlap; y++)  {
                int idx = x*overlap + y;

                Vec3b a0 = A.at<Vec3b>(y, xoffset + x);
                Vec3b b0 = B.at<Vec3b>(y, x);
                double cap0 = norm(a0, b0);
                
                 // Add bottom edge
                if(y+1 < overlap) {
                    Vec3b a1 = A.at<Vec3b>(yoffset + y + 1, x);
                    Vec3b b1 = B.at<Vec3b>(y + 1,x);
                    double cap1 = norm(a1, b1);
                    #pragma omp critical
                    g.add_edge(idx, idx + 1, (int)(cap0 + cap1), (int)(cap0 + cap1));
                }

                // Add right edge
                if(x+1 < A.cols) {
                    Vec3b a2 = A.at<Vec3b>(yoffset + y, x+1);
                    Vec3b b2 = B.at<Vec3b>(y, x+1);
                    double cap2 = norm(a2, b2);
                    #pragma omp critical
                    g.add_edge(idx, idx + overlap, (int)(cap0 + cap2), (int)(cap0 + cap2));
                }
            }     
        }
    }
    
    int flow = g.maxflow();
    cout << "max flow: " << flow << endl;

    graphcut = no_graphcut.clone();
    graphcut_and_cutline = no_graphcut.clone();

    int idx = 0;
    if (orientation == 1)
    {

        for(int y=0; y < A.rows; y++) {
            for(int x=0; x < overlap; x++) {
                if(g.what_segment(idx) == GraphType::SOURCE) {
                    graphcut.at<Vec3b>(y, xoffset + x) = A.at<Vec3b>(y, xoffset + x);
                }
                else {
                    graphcut.at<Vec3b>(y, xoffset + x) = B.at<Vec3b>(y, x);
                }
                graphcut_and_cutline.at<Vec3b>(y, xoffset + x) =  graphcut.at<Vec3b>(y, xoffset + x);

                idx++;
            }
        }  
    }
    
    if (orientation == 2)
    {
        for(int x=0; x < A.cols; x++) {
            for( int y=0; y < overlap; y++)  {
                if(g.what_segment(idx) == GraphType::SOURCE) {
                    graphcut.at<Vec3b>(yoffset + y, x) = A.at<Vec3b>(yoffset + y, x);
                }
                else {
                    graphcut.at<Vec3b>(yoffset + y, x) = B.at<Vec3b>(y, x);
                }
                graphcut_and_cutline.at<Vec3b>(yoffset + y,  x) =  graphcut.at<Vec3b>(yoffset + y, x);
                idx++;
            }
        }
    }

    imshow("graphcut and cut line", graphcut_and_cutline);
    imshow("graphcut", graphcut);
    return graphcut;
}

Mat FinalImage::selectSubset(Mat &originalImg, int width_patch, int height_patch)
{
    //copy a sub matrix of X to Y with starting coodinate (startX,startY)
    // and dimension (cols,rows)
    int startX = rand_r(&seed)% (originalImg.cols - (width_patch + 10));
    int startY = rand_r(&seed)% (originalImg.rows - (height_patch + 10));
    Mat tmp = originalImg(cv::Rect(startX, startY, width_patch, height_patch)); 
    Mat subset;
    tmp.copyTo(subset);
    return subset;

}

double FinalImage::msqe(Mat &target, Mat &patch)
{
    int i, j;
    double eqm, tmpEqm = 0;
    int height = target.rows;
    int width = target.cols;
    uint8_t* pixelPtrT = (uint8_t*)target.data;
    uint8_t* pixelPtrP = (uint8_t*)patch.data;
    Scalar_<uint8_t> bgrPixelT;
    int cnT = target.channels();
    int cnP = patch.channels();
    double valTarget, valPatch = 0;
    double R, G, B = 0;

    for ( i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {

            B = pixelPtrT[i*target.cols*cnT + j*cnT + 0]; // B
            B -= pixelPtrP[i*patch.cols*cnP + j*cnP + 0]; // B

            G = pixelPtrT[i*target.cols*cnT + j*cnT + 1]; // G
            G -= pixelPtrP[i*patch.cols*cnP + j*cnP + 1]; // G

            R = pixelPtrT[i*target.cols*cnT + j*cnT + 2]; // R
            R -= pixelPtrP[i*patch.cols*cnP + j*cnP + 2]; // R

            tmpEqm = sqrt((B + G + R) * (B + G + R));
            eqm += tmpEqm;

            valTarget = 0;
            valPatch = 0;
          
        }
    }

    eqm /= height * width;
    return eqm;
}

Patch FinalImage::getRandomPatch(std::vector<Patch> patchesList)
{
    //return random error from list of best errors
    //If we don't do this, the chosen patches will look extremely similar
    std::vector<Patch> bestErrorsList;

    double tempError;
    Patch bestPatch; 
    double minError = 2.0; //Chose a value for acceptable error
    
    //Check that each new error stored in PatchesList is in fact new
    std::vector<Patch>::iterator isRepeatedElem;
    bool repeatedElem;
    while (bestErrorsList.size() < 50)
    {
        for (int i = 0; i < patchesList.size(); i++)
        {
            isRepeatedElem = find_if(bestErrorsList.begin(), bestErrorsList.end(), findRepeatedPatch(patchesList[i].error));
            if (isRepeatedElem != bestErrorsList.end()) repeatedElem = true;
            else repeatedElem = false;

            if (patchesList[i].error < minError && !repeatedElem )
            {
                bestErrorsList.push_back(patchesList[i]);
            }
        }
        minError += 3;
    }


    int i = rand() % (bestErrorsList.size() - 1); //return random error from list of best errors
    if ( bestErrorsList.size() != 0) {    
        bestPatch = bestErrorsList[i];
    }
    if (bestErrorsList.size() == 0)
        cout << "WARNING, empty list" << endl; //This should never happen

    return bestPatch;
}

Mat FinalImage::placeRandomly(Patch patch, Mat &img)
{
    int posXPatch =  0;
    int posYPatch = 0;
    srand(time(NULL)); //Seed to get randmon patches


    for (int patchesInY = 0; patchesInY < newimg.rows/patch.height; patchesInY++)
   {
        for (int patchesInX = 0; patchesInX < newimg.cols/patch.width; patchesInX++)
        {
            patch.image = selectSubset(img, patch.width, patch.height); //subselection from original texture
            Rect rect2(posXPatch, posYPatch, patch.width, patch.height);
            patch.image.copyTo(newimg(rect2));
            posXPatch += patch.width;
        }
        posXPatch = 0;
        posYPatch += patch.height;
    }
   
   return newimg;

}

Mat FinalImage::choseTypeTexture(Mat &img, Mat &img2, Mat &img3, Patch &p, Grid &g, int x, int y) //Chose either background (0) or details (1) texutre
{
    if (g.grid[x][y] == 0){
        p.typeOfTexture = 0;
        return img;
    }
    else if (g.grid[x][y] == 1){
        p.typeOfTexture = 1;
        return img2;
    }
    else if (g.grid[x][y] == 2){
        p.typeOfTexture = 2;
        return img3;
    }   
}

Mat FinalImage::addBlending(Mat &_patch, Mat &_template, Point center)
{
    Mat src = _patch;
    Mat dst = _template(Rect(0, 0, _template.cols, _template.rows));
    // Create an all white mask
    Mat src_mask = 255 * Mat::ones(src.rows, src.cols, src.depth());
    Mat normal_clone;
    seamlessClone(src, dst, src_mask, center, normal_clone, NORMAL_CLONE); 
    return normal_clone;
}

Mat FinalImage::textureSynthesis(Patch patch, Patch target, Mat &img, Mat &img2, Mat &img3, int backgroundPorcentage, int detailsPorcentage)
{
    Patch newTarget(img); //Target
    Patch bestP(img);     //Patch

    Point center;

    //Mats for cloning
    Mat src;
    Mat dst;
    Mat normal_clone;

    Mat selectedTexture, _newImg, _finalImage;

    overlap = patch.width / 6; 
    offset = patch.width - overlap; 
    posYPatch = posYTarget = 0;
    posXPatch = patch.width - overlap;
    gridSize = (newimg.cols/patch.width) * (newimg.rows/patch.height);

    //Size of grid
    gridX = (width / patch.width) + 1; //plus one because with the overlaping of patches, there is space for one more
    gridY = (height / patch.height) ;

    //Create grid with random distribution for either background or texture
    Grid grid(gridX, gridY); 
    grid.fill(backgroundPorcentage, detailsPorcentage); 

    //Create first target
    selectedTexture = choseTypeTexture(img, img2, img3, patch, grid, 0,0); 
    target.image = selectSubset(selectedTexture, target.width, target.height); //Create a smaller subset of the original image 
    Rect rect(0,0, target.width, target.height);
    target.image.copyTo(newimg(rect));
     //_patchesList.resize(101);
    
    for (int patchesInY = 0; patchesInY < grid.grid[1].size(); patchesInY++)
   {
        for (int patchesInX = 1; patchesInX < grid.grid.size(); patchesInX++) 
        {    
            //Choose texture background or foreground 
            selectedTexture = choseTypeTexture(img, img2, img3, patch, grid, patchesInX, patchesInY);
           
            //seed to get random patch from input
            seed = omp_get_thread_num();
            
            ///Start comparing patches (until error is lower than tolerance)  
            #pragma omp parallel
            { 
                //Private objects to parallelize
                std::vector<Patch> _patchesList_private;
                Patch _patch(img); 

               #pragma omp for 
                for (int i = 0; i < 60 ; i++) //This value needs to be at least 50
                {          
                   //Set image to patch
                    _patch.image = selectSubset(selectedTexture, patch.width, patch.height); //subselection from original texture
          
                    //Create ROIs
                    _patch.roiOfPatch = _patch.image(Rect(0, 0, overlap, _patch.height));
                    _patch.roiOfTarget = target.image(Rect(offset, 0, overlap, target.height));
                    _patch.halfOfTarget = target.image(Rect(target.width/4, 0, target.width-(target.width/4), target.height));

                    //Get MSQE
                    err = msqe(_patch.roiOfTarget, _patch.roiOfPatch); 

                    //if is the second or bigger row
                    if (patchesInY > 0) 
                    {
                        _patch.roiOfTopPatch = _patch.image(Rect(0, 0, _patch.width, overlap));
                        _patch.roiOfBotTarget = newimg(Rect(posXPatch, posYPatch - overlap, _patch.width, overlap));
                        err += msqe(_patch.roiOfTopPatch, _patch.roiOfBotTarget);
                        err = err/2; 
                    }

                    //Set values to patch
                    _patch.error = err;
                    _patchesList_private.push_back(_patch); 

                    //Reset error for next iteration
                    err = 0;
                }
                //Join parallel vectors
                #pragma omp critical
                _patchesList.insert(_patchesList.end(), _patchesList_private.begin(), _patchesList_private.end());
            }
  
            //chose random patch from best errors list
            bestP = getRandomPatch(_patchesList); 
            _newImg = newimg(Rect(0, posYPatch, posXPatch + overlap, bestP.image.rows)); //temporal target
            _newImg = graph_Cut( _newImg, bestP.image, overlap, 1);
            _newImg.copyTo(newimg(Rect(0, posYPatch, _newImg.cols, _newImg.rows)));

            //Set new target, which is the best patch of this iteration       
            target.image = bestP.image;
            posXPatch += patch.width - overlap;
            _patchesList.clear(); 
        }   


        posXPatch = patch.width - overlap; //Update posision of X
        posYPatch += patch.height;//New patch of the next row (which is the new first target)
        newTarget.roiOfBotTarget = newimg(Rect(0, posYPatch - overlap , patch.width, overlap));
        if (patchesInY + 1 != grid.grid[1].size())
        {
            for (int i = 0; i < 100; i++)
            {
                newTarget.image = selectSubset(img, newTarget.width, newTarget.height); //subselection from original texture            
                //Create ROIs
                newTarget.roiOfTopPatch = newTarget.image(Rect(0, 0, newTarget.width, overlap));   
                //Calculate errors
                err = msqe(newTarget.roiOfTopPatch, newTarget.roiOfBotTarget);
                newTarget.error = err;
                _patchesList.push_back(newTarget);
            }
            //chose best error
            bestP = getRandomPatch(_patchesList);
            newTarget.image = bestP.image;
            target = newTarget;
            target.image.copyTo(newimg(Rect(0, posYPatch, target.image.cols, target.image.rows)));
            _patchesList.clear();     
        }
        
    }
    posYPatch = 0;
    posXPatch = patch.width - overlap;
    int widht_Final_image = newimg.cols - overlap * 2;
    Mat _patch, _template, gc, synthesised_Image;
    int newTmpY = 0; //new position in y to do the GC

    synthesised_Image = Mat::zeros(newimg.rows, widht_Final_image, CV_64FC3);
    synthesised_Image.convertTo(synthesised_Image, CV_8UC1);
    _template = newimg(Rect(0,posYPatch, widht_Final_image, patch.height));
    _patch = newimg(Rect(0,posYPatch + patch.height, widht_Final_image, patch.height)); 
    
    //#pragma omp parallel for 
    for (int patchesInY = 0; patchesInY <grid.grid[1].size()-1 ; patchesInY++)
    {
        //cout << "threads " << omp_get_num_threads() << endl;
        if (patchesInY != 0)
        {
            _template = synthesised_Image(Rect(0,posYPatch - (overlap * patchesInY), widht_Final_image, patch.height));
            _patch = newimg(Rect(0,posYPatch + patch.height, widht_Final_image, patch.height)); 
        }

        //Apply GC
        gc = graph_Cut(_template, _patch, overlap, 2);
        //imshow("gc", gc);
        gc.copyTo(synthesised_Image(Rect(0,newTmpY, gc.cols, gc.rows)));

        //Apply blending
        // The location of the center of the src in the dst
        Point center(_patch.cols/2 , _patch.rows + overlap);
        normal_clone = addBlending(_patch, gc, center);
        normal_clone.copyTo(gc(Rect(0, 0, normal_clone.cols, normal_clone.rows)));

        //Add the cut + blending to final image
        gc.copyTo(synthesised_Image(Rect(0,newTmpY, gc.cols, gc.rows)));
        _finalImage = synthesised_Image(Rect(0,0, synthesised_Image.cols, (grid.grid[1].size() * patch.height) - (overlap * grid.grid[1].size()-2) ));

        posYPatch += patch.height;
        newTmpY += patch.height - overlap;
        imwrite("patch.jpg", _patch);
        imwrite("template.jpg", _template);
        
    }
    
    imwrite("final.jpg", _finalImage);
    return _finalImage;
}