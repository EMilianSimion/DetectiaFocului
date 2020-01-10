#pragma once

#include <vector> 
#include <iostream> 
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/core.hpp>

using namespace cv;
using namespace std;

typedef unsigned char byte;

#define CSPACE_HSV	 0
#define CSPACE_YCbCr 1
#define KEY_ESC 27

Mat computeROI(Mat inputMatrix, double thresh, int type);
Mat computeMovement(Mat inputMatrix, Mat firstFrame, Mat secondFrame);
Mat computeBBox(Mat inputMatrix);
Mat mClose(Mat inputMatrix);