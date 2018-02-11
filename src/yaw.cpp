#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>

using namespace std;
using namespace cv;

void printUsage()
{
    cout << "[Usage]: ./yaw path/to/video" << endl;
}

double findMedian(vector<double> values)
{
    size_t size = values.size();

    if (size == 0)
    {
        return 0;
    }
    else if (size == 1)
    {
        return values[0];
    }
    else
    {
        sort(values.begin(), values.end());
        if (size % 2 == 0)
        {
            return (values[size / 2 - 1] + values[size / 2]) / 2;
        }
        else
        {
            return values[size / 2];
        }
    }
}


vector<int> filterOutliers(vector<Vec4i> lines, vector<double> angles)
{
    vector<int> filteredIndexes;

    double medianAngle = findMedian(angles);
    /*double med_dev;
    for(int i=0;i<angles.size();i++)
    {
        med_dev += pow(angles[i] - medianAngle,2);
    }
    med_dev = med_dev / angles.size();
    med_dev = sqrt(med_dev);*/
    for(int i=0;i<angles.size();i++)
    {
        // cout<<angles[i]<<" "<<med_dev<<endl;
        if(abs(angles[i] - medianAngle) < 2)
        {
            filteredIndexes.push_back(i);
        }
    }

    return filteredIndexes;
}
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printUsage();
        return -1;
    }

    VideoCapture cap(argv[1]);
    if(!cap.isOpened())
    {
        cout << "Couldn't open video. Exiting!" << endl;
        return -1;
    }

    Mat src, dst, cdst, orig;
    bool paused = false;
    int canny_low = 10, canny_high = 100, threshold = 50, minLineLength = 50, maxLineGap = 10;

    namedWindow("Trackbars");
    createTrackbar( "Canny low", "Trackbars", &canny_low, 500, NULL);
    createTrackbar( "Canny high", "Trackbars", &canny_high, 500, NULL);
    createTrackbar( "threshold", "Trackbars", &threshold, 200, NULL);
    createTrackbar( "minLineLength", "Trackbars", &minLineLength, 200, NULL);
    createTrackbar( "maxLineGap", "Trackbars", &maxLineGap, 200, NULL);

    while (1)
    {
        if(!paused)
        {
            cap >> orig;
        }
        if(orig.empty())
            break;
        //imshow("Video",src);
        medianBlur( orig, src, 5);
        Canny(src, dst, canny_low, canny_high, 3);
        imshow("Canny",dst);
        char c = waitKey(1);
        cvtColor(dst, cdst, CV_GRAY2BGR);
        if(c==27)
            break;
        if(c=='p')
            paused = !paused;

        vector<Vec4i> lines, filteredLinesV, filteredLinesH;
        vector<double> anglesV, anglesH;
        vector<int> filteredIndexesV, filteredIndexesH;

        HoughLinesP(dst, lines, 1, CV_PI/180, threshold, minLineLength, maxLineGap);
        for( size_t i = 0; i < lines.size(); i++ )
        {
            Vec4i l = lines[i];
            double angle = atan2 (abs(l[1]-l[3]),abs(l[0]-l[2])) * 180 / M_PI;
            // since abs is used, this angle can be used only for filtering and
            // not final computation.
            if(abs(90.0-angle) <= 45)
            {
                //line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
                filteredLinesV.push_back(l);
                anglesV.push_back(angle);
            }
            if(abs(angle) < 45)
            {
                //line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255,0,0), 3, CV_AA);
                filteredLinesH.push_back(l);
                anglesH.push_back(angle);
            }
        }

        filteredIndexesH = filterOutliers(filteredLinesH, anglesH);
        filteredIndexesV = filterOutliers(filteredLinesV, anglesV);
        int angleCount = filteredIndexesH.size() + filteredIndexesV.size();
        double currentHeading = 0;
        for(int i=0;i<filteredIndexesH.size();i++)
        {
            Vec4i l = filteredLinesH[filteredIndexesH[i]];
            double angle = atan2 ((l[1]-l[3]),(l[0]-l[2])) * 180 / M_PI;
            if(angle<45 && angle>-45)
                currentHeading += angle;
            else if (angle>135)
                currentHeading += (angle - 180);
            else
                currentHeading += (angle + 180);

            line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
        }

        for(int i=0;i<filteredIndexesV.size();i++)
        {
            Vec4i l = filteredLinesV[filteredIndexesV[i]];
            double angle = atan2 ((l[1]-l[3]),(l[0]-l[2])) * 180 / M_PI;
            if(angle>=45 && angle<=135)
                currentHeading += (angle - 90);
            else
                currentHeading += (angle + 90);
            line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255,0,0), 3, CV_AA);
        }
        currentHeading /= angleCount;
        cout<<currentHeading<<endl;
        imshow("Lines",cdst);

    }
    return 0;
}
