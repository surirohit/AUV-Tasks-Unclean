#include <iostream>
#include <opencv2/opencv.hpp>
#include "GateFinal.h"

void printUsage()
{
    cout << "[Usage]: ./gate path/to/video" << endl;
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

    GateFinal ob1;
    bool paused = false;
    Mat orig;

    while (1)
    {
        if(!paused)
            cap >> orig;

        if(orig.empty())
            break;

        ob1.detectGate(orig);

        char c = waitKey(1);
        if(c==27)
            break;
        if(c==' ')
            paused = !paused;

    }
}
