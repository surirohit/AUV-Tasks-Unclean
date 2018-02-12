#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void printUsage()
{
    cout << "[Usage]: ./gate path/to/video" << endl;
}

void on_trackbar( int, void* )
{
    return;
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
    createTrackbar( "Canny low", "Trackbars", &canny_low, 500, on_trackbar);
    createTrackbar( "Canny high", "Trackbars", &canny_high, 500, on_trackbar);
    createTrackbar( "threshold", "Trackbars", &threshold, 200, on_trackbar);
    createTrackbar( "minLineLength", "Trackbars", &minLineLength, 200, on_trackbar);
    createTrackbar( "maxLineGap", "Trackbars", &maxLineGap, 200, on_trackbar);

    int N = 3;
    vector<vector<Vec4i> > lastNFrameLines(N);
    int curIndex = -1;
    Point gateCenter;
    while (1)
    {
        if(!paused)
        {
            cap >> orig;
            curIndex = (curIndex+1)%N;
        }
        if(orig.empty())
            break;
        //imshow("Video",src);
        medianBlur( orig, src, 5);
        Canny(src, dst, canny_low, canny_high, 3);
        imshow("Canny",dst);
        cvtColor(dst, cdst, CV_GRAY2BGR);

        if(0)
        {
            vector<Vec2f> lines;
            HoughLines(dst, lines, 1, CV_PI/180, threshold, 0, 0 );
            for( size_t i = 0; i < lines.size(); i++ )
            {
                float rho = lines[i][0], theta = lines[i][1];
                Point pt1, pt2;
                double a = cos(theta), b = sin(theta);
                double x0 = a*rho, y0 = b*rho;
                pt1.x = cvRound(x0 + 1000*(-b));
                pt1.y = cvRound(y0 + 1000*(a));
                pt2.x = cvRound(x0 - 1000*(-b));
                pt2.y = cvRound(y0 - 1000*(a));
                double angle = atan2 (abs(pt2.y-pt1.y),abs(pt2.x-pt1.x)) * 180 / M_PI;
                if(abs(90.0-angle)<3)
                    line( cdst, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
                if(abs(angle)<3)
                    line( cdst, pt1, pt2, Scalar(255,0,0), 3, CV_AA);

            }
        }
        else
        {
            vector<Vec4i> lines, filteredLinesV, filteredLinesH;
            HoughLinesP(dst, lines, 1, CV_PI/180, threshold, minLineLength, maxLineGap);
            for( size_t i = 0; i < lines.size(); i++ )
            {
                Vec4i l = lines[i];
                double angle = atan2 (abs(l[1]-l[3]),abs(l[0]-l[2])) * 180 / M_PI;
                if(abs(90.0-angle) < 5)
                {
                    //line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
                    filteredLinesV.push_back(l);
                }
                if(abs(angle) < 5)
                {
                    //line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255,0,0), 3, CV_AA);
                    filteredLinesH.push_back(l);
                }
            }
            lastNFrameLines[curIndex] = filteredLinesV;
        }
        char c = waitKey(20);
        if(c==27)
            break;
        if(c=='p')
            paused = !paused;

        Mat blankMat = Mat::zeros(orig.size(), CV_8UC3);
        for(int i=0;i<N;i++)
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
        //imshow("Gate",blankMat);

        vector<vector<Point> > contours_poly( contours.size() );
        vector<Rect> boundRect( contours.size() );
        for( int i = 0; i < contours.size(); i++ )
        {
            approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
            boundRect[i] = boundingRect( Mat(contours_poly[i]) );
        }

        Mat drawing = Mat::zeros( blankMat.size(), CV_8UC3 );
        for( int i = 0; i< contours.size(); i++ )
        {
            Scalar color = Scalar(0,0,255);
            rectangle( cdst, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
        }

        int index1 = -1, index2 = -1;
        int maxY = -1;
        for(int i=0; i<boundRect.size();i++)
        {
            for(int j=i+1; j<boundRect.size(); j++)
            {
                //calculate highest y coord
                double angle = atan2 (abs(boundRect[i].tl().y-boundRect[j].tl().y),abs(boundRect[i].tl().x-boundRect[j].tl().x)) * 180 / M_PI;
                std::cerr << angle << '\n';
                if(abs(angle)<5)
                {
                    int thisMaxY = (boundRect[i].y>boundRect[j].y)?boundRect[i].y:boundRect[j].y;
                    if(thisMaxY>maxY)
                    {
                        maxY = thisMaxY;
                        index1 = i;
                        index2 = j;
                    }
                }
            }
        }
        if(index1!=-1)
        {
            Scalar color = Scalar(0,0,255);
            bool isIndex1Left = (boundRect[index1].x<boundRect[index2].x)?1:0;
            if(isIndex1Left)
            {
                gateCenter.x = (boundRect[index1].tl().x+boundRect[index2].br().x)/2;
                gateCenter.y = (boundRect[index1].tl().y+boundRect[index2].br().y)/2;
                rectangle( src, boundRect[index1].tl(), boundRect[index2].br(), color, 2, 8, 0 );
            }
            else
            {
                gateCenter.x = (boundRect[index2].tl().x+boundRect[index1].br().x)/2;
                gateCenter.y = (boundRect[index2].tl().y+boundRect[index1].br().y)/2;
                rectangle( src, boundRect[index2].tl(), boundRect[index1].br(), color, 2, 8, 0 );
            }
            line( src, Point(src.cols/2,0), Point(src.cols/2, src.rows), Scalar(255,0,0), 5, CV_AA);
            circle(src, gateCenter, 5, color);
            imshow("Contours", src);
        }
    }
    return 0;
}
