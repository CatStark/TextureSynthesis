#pragma once

#ifndef FinalImage_H
#define FinalImage_H

#include "Grid.h"


class FinalImage
{
public:
    FinalImage(Mat &img, int, int, int);
    Mat selectSubset(Mat &originalImg, int, int);
    Mat placeRandomly(Patch, Mat &img);
    Mat textureSynthesis(Patch patch, Patch target, Mat &img, Mat &img2, Mat &img3, int backgroundPorcentage, int detailsPorcentage, int mode);
    double msqe(Mat &target, Mat &patch);
    Patch getRandomPatch(std::vector<Patch> patchesList);
    Mat choseTypeTexture( Mat &img, Mat &img2, Mat &img3, Patch &p, Grid &g, int, int);
    void addLinearBlending(Mat &target, Mat &patch, int posXPatch, int posYPatch);
    void GC(Mat &source);
    Mat graph_Cut(Mat& A, Mat& B, int overlap, int orientation);
    Mat addBlending(Mat &_patch, Mat &_template,Point center);
    bool circleComplete(Mat &img);


    std::vector<Patch> _patchesList;
    _patches tmpPatch;
    int posYTarget, posYPatch, posXPatch;
    int overlap,offset ;

    Mat newimg;
    double err;
    std::pair<double, Mat> bestError;
    double tempError;
    double minError;
    int width, height;
    int backgroundPorcentageTmp;
    int gridSize, gridX, gridY;
    unsigned int seed;
private:
};

#endif