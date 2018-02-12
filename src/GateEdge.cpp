#include "GateEdge.h"

GateEdge::GateEdge()
{
    N = 5;
    curIndex = -1;
    cannyLow = 10, cannyHigh = 50, threshold = 10, minLineLength = 30, maxLineGap = 5;
    nCenters = 100;
    nCentersIndex = -1;
};

GateEdge::~GateEdge(){};

bool GateEdge::detectGate(Mat orig)
{
    Mat src, dst, cdst;
    curIndex = (curIndex + 1)%N;

    medianBlur( orig, src, 5);
    Canny(src, dst, cannyLow, cannyHigh, 3);
    imshow("Canny gate",dst);
    cvtColor(dst, cdst, CV_GRAY2BGR);

    vector<Vec4i> lines, filteredLinesV, filteredLinesH;
    HoughLinesP(dst, lines, 1, CV_PI/180, threshold, minLineLength, maxLineGap);
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        double angle = atan2 (abs(l[1]-l[3]),abs(l[0]-l[2])) * 180 / M_PI;
        if(abs(90.0-angle) < 5)
        {
            filteredLinesV.push_back(l);
        }
        if(abs(angle) < 5)
        {
            filteredLinesH.push_back(l);
        }
    }

    if(lastNFrameLines.size()==N)
        lastNFrameLines[curIndex] = filteredLinesV;
    else
        lastNFrameLines.push_back(filteredLinesV);

    Mat blankMat = Mat::zeros(orig.size(), CV_8UC3);
    for(int i=0;i<lastNFrameLines.size();i++)
    {
        for(int j=0;j<lastNFrameLines[i].size();j++)
        {
            Vec4i l = lastNFrameLines[i][j];
            line( blankMat, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255,255,255), 5, CV_AA);
        }
    }

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    cvtColor(blankMat.clone(),blankMat,CV_BGR2GRAY);
    findContours( blankMat, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
    imshow("Gate",blankMat);

    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
    }

    // First filtering:
    // 1. Check if top and bottom lines are approximately straight
    // 2. Check width and height
    // 3. Store in vector

    vector<Rect> filteredSquares;
    int width, height;
    for(int i=0; i<boundRect.size();i++)
    {
        for(int j=i+1; j<boundRect.size(); j++)
        {
            //calculate highest y coord
            double angle1 = atan2 (abs(boundRect[i].tl().y-boundRect[j].tl().y),abs(boundRect[i].tl().x-boundRect[j].tl().x)) * 180 / M_PI;
            double angle2 = atan2 (abs(boundRect[i].br().y-boundRect[j].br().y),abs(boundRect[i].br().x-boundRect[j].br().x)) * 180 / M_PI;
            if(abs(angle1)<5 && abs(angle2)<5)
            {
                bool isFirstLeft = (boundRect[i].x<boundRect[j].x)?1:0;
                height = max(boundRect[j].br().y,boundRect[i].br().y) - min(boundRect[j].tl().y,boundRect[i].tl().y);

                if(isFirstLeft)
                    width = boundRect[j].br().x - boundRect[i].tl().x;
                else
                    width = boundRect[i].br().x - boundRect[j].tl().x;

                if(height*1.3<width || width*1.3<height)
                    continue;

                if(isFirstLeft)
                    filteredSquares.push_back(Rect(boundRect[i].tl().x,min(boundRect[j].tl().y,boundRect[i].tl().y),width,height));
                else
                    filteredSquares.push_back(Rect(boundRect[j].tl().x,min(boundRect[j].tl().y,boundRect[i].tl().y),width,height));
            }
        }
    }

    if(filteredSquares.size()==0)
        return false;

    // If there is nothing to compare to and more than 1 possible gate is detected
    if(lastCenters.size() == 0 && filteredSquares.size()!=1)
        return false;

    if(lastCenters.size() == 0)
    {
        Rect r = filteredSquares[0];
        lastCenters.push_back(Point(r.x+r.width/2,r.y+r.height/2));
    }

    int index = 0;
    double minDev = DBL_MAX;

    for(int i=0;i<filteredSquares.size();i++)
    {
        double dev = 0;
        Rect r = filteredSquares[0];
        Point p1 = Point(r.x+r.width/2,r.y+r.height/2);
        for(int j=0;j<lastCenters.size();j++)
        {
            Point p2 = lastCenters[j];
            dev+= dist(p1,p2);
        }
        if(dev<minDev)
        {
            minDev = dev;
            index = i;
        }
    }

    gateRect = filteredSquares[index];
    gateCenter = Point(gateRect.x+gateRect.width/2,gateRect.y+gateRect.height/2);

    nCentersIndex = (nCentersIndex+1)%nCenters;

    if(lastCenters.size()<nCenters)
    {
        lastCenters.push_back(gateCenter);
        return false;
    }
    else
    {
        lastCenters[nCentersIndex] = gateCenter;
        int count = 0;
        for(int i=0;i<lastCenters.size();i++)
        {
            cout<<dist(gateCenter,lastCenters[i])<<endl;
            if(dist(gateCenter,lastCenters[i])<100)
                count++;
        }
        if(count<0.6*nCenters)
            return false;

        rectangle( cdst, gateRect, Scalar(0,0,255), 2, 8, 0 );
        imshow("Cdst",cdst);

        return true;
    }
}

double GateEdge::dist(Point p1,Point p2)
{
    return sqrt(pow(p1.x-p2.x,2)+pow(p1.y-p2.y,2));
}
