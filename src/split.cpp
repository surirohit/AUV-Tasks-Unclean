#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void printUsage()
{
    cout << "[Usage]: ./split path/to/video" << endl;
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

    bool paused = false;
    Mat orig;
    while (1)
    {
        if(!paused){
            cap >> orig;
            cvtColor(orig.clone(), orig, CV_BGR2HSV);
        }
        if(orig.empty())
            break;
        Mat bgr[3];
        Mat otsuOut[3];
        split(orig,bgr);
        // cv::threshold(bgr[0], otsuOut[0], 100, 255, CV_THRESH_BINARY_INV );//| CV_THRESH_OTSU);
        // cv::threshold(bgr[1], otsuOut[1], 100, 255, CV_THRESH_BINARY_INV );//| CV_THRESH_OTSU);
        // cv::threshold(bgr[2], otsuOut[2], 100, 255, CV_THRESH_BINARY_INV );//| CV_THRESH_OTSU);
        imshow("Blue" ,bgr[0]);
        imshow("Green",bgr[1]);
        imshow("Red"  ,bgr[2]);

        char c = waitKey(20);
        if(c==27)
            break;
        if(c=='p')
            paused = !paused;
    }

    return 0;
}
