#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

int sobelThreshold = 4;
int morph_size = 1;

int main(int argc, char **argv){
	VideoCapture cap(argv[1]);
	if(!cap.isOpened()) return -1;

	namedWindow("Trackbars");
	createTrackbar( "Sobel threshold", "Trackbars", &sobelThreshold, 255, NULL);
    createTrackbar( "Kernel size:", "Trackbars", &morph_size, 10, NULL );
	Mat src, temp;
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;
    /// Generate grad_x and grad_y

    Mat grad_x, grad_y,grad,src_gray;
    Mat abs_grad_x, abs_grad_y;
    bool paused = false;
    Mat orig;

	while(cap.isOpened()){
        double timer = (double)getTickCount();
        Mat element = getStructuringElement( MORPH_ELLIPSE, Size( 2*morph_size + 1, 2*morph_size+1 ));

        if(!paused)
		cap >> orig;

        char c= waitKey(1);
		if(c == ' ')
			paused = !paused;
        if(c==27)
            break;
        if(orig.empty())
			break;

        GaussianBlur( orig, src, Size(3,3), 0, 0, BORDER_DEFAULT );

        /// Convert it to gray
        cvtColor( src, src_gray, CV_BGR2GRAY );

        /// Gradient X
        //Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
        Sobel( src_gray, grad_x, ddepth, 2, 0, 1, scale, delta, BORDER_DEFAULT );
        convertScaleAbs( grad_x, abs_grad_x );

        /// Gradient Y
        //Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
        Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
        convertScaleAbs( grad_y, abs_grad_y );

        /// Total Gradient (approximate)
        // addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );

        imshow("W",abs_grad_y);
        dilate(abs_grad_x,abs_grad_x,element);
        imshow("W1",abs_grad_x);

        threshold(abs_grad_x,abs_grad_x,sobelThreshold,255, THRESH_BINARY);
        vector<Vec4i> lines;
    	HoughLinesP(abs_grad_x, lines, 1, CV_PI/180, 50, 50, 10 );
    	for( size_t i = 0; i < lines.size(); i++ )
    	{
            Vec4i l = lines[i];
            double angle = atan2 (abs(l[1]-l[3]),abs(l[0]-l[2])) * 180 / M_PI;
            if(abs(90.0-angle) < 5)
            {
                line( src, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
                //filteredLinesV.push_back(l);
            }

    	}
        float fps = getTickFrequency() / ((double)getTickCount() - timer);
        cout<<fps<<endl;
        imshow("orig",src);
        imshow( "g2", abs_grad_x );
	}


return 0;
}
