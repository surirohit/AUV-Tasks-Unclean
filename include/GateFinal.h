#include <opencv2/opencv.hpp>
#include <iostream>
#include <limits.h>

using namespace cv;
using namespace std;

class GateFinal
{
public:
    GateFinal();
	~GateFinal();
    bool detectGate(Mat orig);
    double dist(Point p1, Point p2);

    int nCenters, nCentersIndex;
    vector<Point> lastCenters;
    Point gateCenter;
    Rect gateRect;
    int scale, delta, ddepth;
    int sobelThreshold, morph_size;

    Mat src, temp, cdst;
    Mat grad_x,grad_y,src_gray;
    Mat abs_grad_x,abs_grad_y;
};
