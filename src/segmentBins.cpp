#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void printUsage()
{
    cout << "[Usage]: ./segment path/to/video" << endl;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printUsage();
        return -1;
    }

    VideoCapture cap(argv[1]);

    int HL=15,HH=165, SL=0, SH=255, VL=0,VH=47;
    namedWindow("Trackbars");
    createTrackbar( "Hue low", "Trackbars",  &HL, 180, NULL);
    createTrackbar( "Hue high", "Trackbars", &HH, 180, NULL);
    createTrackbar( "Saturation low", "Trackbars",  &SL, 255, NULL);
    createTrackbar( "Saturation high", "Trackbars", &SH, 255, NULL);
    createTrackbar( "Value low", "Trackbars",  &VL, 255, NULL);
    createTrackbar( "Value high", "Trackbars", &VH, 255, NULL);

    if(!cap.isOpened())
    {
        cout << "Couldn't open video. Exiting!" << endl;
        return -1;
    }

    bool paused = false;
    Mat orig,hsv,frame;
    while (1)
    {

        if(!paused)
        {
            cap >> orig;
            if(orig.empty())
            break;
            cvtColor(orig,hsv, CV_BGR2HSV);
        }

        imshow("Image",hsv);
        frame = hsv.clone();
        Mat thresh1, thresh2, thresh;
        inRange(frame,Scalar(0,SL,VL), Scalar(HL,SH,VH),thresh1);
        inRange(frame,Scalar(HH,SL,VL), Scalar(180,SH,VH),thresh2);
        bitwise_or(thresh1,thresh2,thresh);
        Rect r = Rect(Point(150,244),Point(500,400));
        Mat crop = thresh(r);
        Mat element = getStructuringElement( MORPH_ELLIPSE, Size( 3, 3 ));
        dilate(crop,crop,element);
        cvtColor(thresh,thresh,CV_GRAY2BGR);
        rectangle(thresh,r,Scalar(0,255,0),2,8,0);

        imshow("Frame",thresh);

        frame = orig.clone();
        medianBlur( frame.clone(), frame, 5);
        Canny(frame.clone(), frame, 1500, 4500, 5);
        imshow("Canny gate",frame);

        char c = waitKey(1);
        if(c==27)
            break;
        if(c==' ')
            paused = !paused;

    }
}
