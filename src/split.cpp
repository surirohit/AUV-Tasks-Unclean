#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstring>

using namespace std;
using namespace cv;

void printUsage()
{
    cout << "[Usage]: ./split path/to/video" << endl;
}

void rgb2cmyk(cv::Mat& img, std::vector<cv::Mat>& cmyk) {
    // Allocate cmyk to store 4 componets
    for (int i = 0; i < 4; i++) {
        cmyk.push_back(cv::Mat(img.size(), CV_8UC1));
    }

    // Get rgb
    std::vector<cv::Mat> rgb;
    cv::split(img, rgb);

    // rgb to cmyk
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            float r = (int)rgb[2].at<uchar>(i, j) / 255.;
            float g = (int)rgb[1].at<uchar>(i, j) / 255.;
            float b = (int)rgb[0].at<uchar>(i, j) / 255.;
            float k = std::min(std::min(1- r, 1- g), 1- b);

            cmyk[0].at<uchar>(i, j) = (1 - r - k) / (1 - k) * 255.;
            cmyk[1].at<uchar>(i, j) = (1 - g - k) / (1 - k) * 255.;
            cmyk[2].at<uchar>(i, j) = (1 - b - k) / (1 - k) * 255.;
            cmyk[3].at<uchar>(i, j) = k * 255.;
        }
    }
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
            if(argc==3 && argv[2][0]=='H')
                cvtColor(orig.clone(), orig, CV_BGR2HSV);
        }
        if(argc==3 && argv[2][0]=='C')
        {
            std::vector<cv::Mat> dst;
            rgb2cmyk(orig, dst);

            // Display results
            cv::imshow("src", orig);
            cv::imshow("c", dst[0]);
            cv::imshow("m", dst[1]);
            cv::imshow("y", dst[2]);
            cv::imshow("k", dst[3]);
            char c = waitKey(20);
            if(c==27)
                break;
            if(c=='p')
                paused = !paused;
            continue;
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
