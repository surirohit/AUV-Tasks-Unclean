#include "GateFinal.h"

GateFinal::GateFinal()
{
    sobelThreshold = 50;
    morph_size = 1;
    nCenters = 50;
    nCentersIndex = -1;
    scale = 1;
    delta = 0;
    ddepth = CV_16S;
};

GateFinal::~GateFinal(){}

bool GateFinal::detectGate(Mat orig)
{
    // le some basic shit
    Mat element = getStructuringElement( MORPH_ELLIPSE, Size( 2*morph_size + 1, 2*morph_size+1 ));

	GaussianBlur( orig, src, Size(3,3), 0, 0, BORDER_DEFAULT );

	cvtColor( src, src_gray, CV_BGR2GRAY );

	Sobel( src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
	convertScaleAbs( grad_x, abs_grad_x );
    //dilate(abs_grad_x,abs_grad_x,element);


	threshold(abs_grad_x,abs_grad_x,sobelThreshold,255, THRESH_BINARY);
	//imshow("Sobel X Abs",abs_grad_x);

    // Le changes scale if kernel size if needed
    Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
    convertScaleAbs( grad_y, abs_grad_y );
    //dilate(abs_grad_y,abs_grad_y,element);

	abs_grad_y = abs_grad_y - abs_grad_x; //  le removing places where both edges are coming
    threshold(abs_grad_y,abs_grad_y,40,255, THRESH_BINARY); // threshold need to be tuned

    cvtColor(abs_grad_x,cdst,CV_GRAY2BGR);

    // le finding lines and filtering
    // le changing parameters will not do much
    vector<Vec4i> lines,filteredLinesH,filteredLinesV;
	HoughLinesP(abs_grad_x, lines, 1, CV_PI/180, 50, 50, 10 );

	for( int i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        double angle = atan2 (abs(l[1]-l[3]),abs(l[0]-l[2])) * 180 / M_PI;
        if(abs(90.0-angle) < 5)
        {
            filteredLinesV.push_back(l);
        }
    }

    // le drawing wide lines so that contour detection me issue na ho
    // le fucks up sometimes because of noise
    // le merges reflection with original creating big contours
    // le maybe can do something better
    Mat blankMat = Mat::zeros(orig.size(), CV_8UC3);
    for(unsigned int i= 0;i<filteredLinesV.size();i++)
    {
            Vec4i l = filteredLinesV[i];
            line( blankMat, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255,255,255), 5, CV_AA);
    }

    vector<vector<Point> > contours;
    //vector<Vec4i> hierarchy;
    cvtColor(blankMat.clone(),blankMat,CV_BGR2GRAY);
    findContours( blankMat, contours, RETR_LIST, CHAIN_APPROX_SIMPLE, Point(0, 0) );

    // le drawing a rectangle over region which needs to be checked for
    // horizontal bar
    // See this link for cropping technique to prevent error
    // http://answers.opencv.org/question/70953/roi-out-of-bounds-issue/
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size() );
    Mat src_clone = src.clone();
	Mat sobel_y_clone = abs_grad_y.clone();
	cvtColor(sobel_y_clone,sobel_y_clone,CV_GRAY2BGR);

	Mat topBarCheckCrop;
	vector<Rect> filteredSquares;
	for( unsigned int i =  0; i < contours.size(); i++ )
    {
        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
        int height = boundRect[i].height;
        Point tl = Point(boundRect[i].tl().x-0.02*height,boundRect[i].tl().y-0.02*height);
        Point br = Point(boundRect[i].tl().x+1.05*height,boundRect[i].tl().y+0.1*height);
        rectangle( src_clone, boundRect[i], Scalar(0,0,255), 2, 8, 0 );
        rectangle( src_clone, tl,br, Scalar(255,0,0), 2, 8, 0 );
		rectangle( sobel_y_clone, tl,br, Scalar(255,0,0), 2, 8, 0 );
		Rect topBarCheckImgRect = Rect(tl.x,tl.y,br.x-tl.x+1,br.y-tl.y+1) & Rect(0, 0, src.cols, src.rows);
		if(topBarCheckImgRect.width <=0 || topBarCheckImgRect.height <=0 )
			continue;
		 // New check for top bar
		 topBarCheckCrop = abs_grad_y(topBarCheckImgRect);
		 filteredLinesH.clear();
		 lines.clear();
		 HoughLinesP(topBarCheckCrop, lines, 1, CV_PI/180, 50, 50, 10 );
		for(int i = 0; i < lines.size(); i++ )
        {
            Vec4i l = lines[i];
            double angle = atan2 (abs(l[1]-l[3]),abs(l[0]-l[2])) * 180 / M_PI;
            if(abs(angle) < 5)
            {
                filteredLinesH.push_back(l);
            }
        }
		//imshow("Topbar", topBarCheckCrop);
		if(filteredLinesH.size() == 0)
			continue;

		// le take longest line and check width if almost close to height

		int max_line_index = -1;
		int max_line_size = 0;
		for(unsigned int i= 0;i<filteredLinesH.size();i++)
		{
			if(max_line_size < (filteredLinesH[i][2]-filteredLinesH[i][0]) )
			{
				max_line_index = i;
				max_line_size  = (filteredLinesH[i][2]-filteredLinesH[i][0]);
			}
		}
		double barRatio = max_line_size/((float)height);

		// le don't know why but above code of findcontours and approxpoly is not giving any results

		if(barRatio <1.1 && barRatio > 0.8) // le tuned somewhat, still has some false positives
		{
		//	cout << "Here Height " << height << "   LH   " << largestTopbar.height<< "   Width   " << largestTopbar.width << endl;
		//	imshow("Topbar",topBarCheckCrop);
		//    waitKey(0);
			//Rect foundRect = Rect(tl.x,tl.y,largestTopbar.width,height) & Rect(0,0,src.cols,src.rows);
			Rect foundRect = Rect(tl.x,tl.y,(1.1*height),(1.05*height)) & Rect(0,0,src.cols,src.rows);
			rectangle( src_clone, foundRect, Scalar(0,255,0), 2, 8, 0 );
			filteredSquares.push_back(foundRect);
		}
	}
	imshow("Src",src_clone);
	imshow("sobely",sobel_y_clone);

    // nothing passed filtering
    if(filteredSquares.size()==0)
    {
        return false;
    }

    // If there is nothing to compare to and more than 1 possible gate is detected
    if(lastCenters.size() == 0 && filteredSquares.size()!=1)
        return false;

    // dont remember why I did THis
    // will cause no harm probably
    if(lastCenters.size() == 0)
    {
        Rect r = filteredSquares[0];
        lastCenters.push_back(Point(r.x+r.width/2,r.y+r.height/2));
    }

    int index = 0;
    double minDev = DBL_MAX;

    // finding mimimum deviation from previously detected gate centers
    for(unsigned int i= 0;i<filteredSquares.size();i++)
    {
        double dev = 0;
        Rect r = filteredSquares[0];
        Point p1 = Point(r.x+r.width/2,r.y+r.height/2);
        for(unsigned int j= 0;j<lastCenters.size();j++)
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

    // decided which rectangle to take
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
        // ignore this part
        // will probably remove it
        lastCenters[nCentersIndex] = gateCenter;
        int count = 0;
        for(unsigned int i= 0;i<lastCenters.size();i++)
        {
            if(dist(gateCenter,lastCenters[i])<100)
                count++;
        }
        if(count<0.6*nCenters)
        {
            return false;
        }
        line(cdst,Point(src.cols/2,0),Point(src.cols/2,src.rows),Scalar(0,0,255),2,2,0);
        rectangle( cdst, gateRect, Scalar(255,0,0), 2, 8, 0 );
        circle(cdst,gateCenter,5,Scalar(255,0,0),2,5,0);
        imshow("Cdst",cdst);
        waitKey(1);
        return true;
    }
}

double GateFinal::dist(Point p1,Point p2)
{
    return sqrt(pow(p1.x-p2.x,2)+pow(p1.y-p2.y,2));
}
