#include <opencv2/opencv.hpp>
#include <iostream>
#include <limits.h>

using namespace cv;
using namespace std;

class GateEdge
{
public:
    GateEdge();
	~GateEdge();
    bool detectGate(Mat orig);
    double dist(Point p1, Point p2);

    int N;
    int nCenters, nCentersIndex;
    vector<vector<Vec4i> > lastNFrameLines;
    vector<Point> lastCenters;
    int curIndex;
    Point gateCenter;
    int cannyLow, cannyHigh, threshold, minLineLength, maxLineGap;
    Rect gateRect;

};
