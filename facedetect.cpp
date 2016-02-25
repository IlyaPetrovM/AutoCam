#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <opencv2/videoio/videoio.hpp>  // Video write

using namespace std;
using namespace cv;

static void help()
{
    cout << "\nThis program demonstrates the cascade recognizer. Now you can use Haar or LBP features.\n"
            "This classifier can recognize many kinds of rigid objects, once the appropriate classifier is trained.\n"
            "It's most known use is for faces.\n"
            "Usage:\n"
            "./facedetect [--cascade=<cascade_path> this is the primary trained classifier such as frontal face]\n"
               "   [--nested-cascade[=nested_cascade_path this an optional secondary classifier such as eyes]]\n"
               "   [--scale=<image scale greater or equal to 1, try 1.3 for example>]\n"
               "   [--try-flip]\n"
               "   [filename|camera_index]\n\n"
            "see facedetect.cmd for one call:\n"
            "./facedetect --cascade=\"../../data/haarcascades/haarcascade_frontalface_alt.xml\" --nested-cascade=\"../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xml\" --scale=1.3\n\n"
            "During execution:\n\tHit any key to quit.\n"
            "\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip );

string cascadeName = "../../data/haarcascades/haarcascade_frontalface_alt.xml";
string cascade2Name = "../../data/haarcascades/haarcascade_profileface.xml";
string nestedCascadeName = "../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
    void updateRoiCoords(vector<Rect> faces,vector<Rect> &rois, int maxCols, int maxRows){
    	static int roiHeight=240,roiWidth=roiHeight*16.0/9.0;
    	static Point tRuleOffsetLeftUp(roiWidth/3.0,roiHeight/3.0);
    	static Point tRuleOffsetLeftDown(roiWidth/3.0, 2.0*roiHeight/3.0);
    	static Point tRuleOffsetRightUp(2.0*roiWidth/3.0,roiHeight/3.0);
    	static Point tRuleOffsetRightDown(2.0*roiWidth/3.0,2.0*roiHeight/3.0);
    	static double t = 0;
    	static int sens=10; //sensivity
    	static float maxSpd=10.0, minSpd=1.0;
 		static float distX=0.0; // distance
 		static float distY=0.0; // distance
 		static float dPrevX=0.0; // distance
 		static float dPrevY=0.0; // distance
 		static float DeltaX=0.0; // distance
 		static float DeltaY=0.0; // distance
		static double Kp=0.1, Ki=0.0, Kd=0.0;
		static double cntX=0.0,cntY=0.0;
		static double uX,uY;	
    	
    	//todo автоматическое увеличение количества кадриков
    	if(rois.empty()){
    		for (int i = 0; i < faces.size(); ++i)
    		{
    			rois.push_back(Rect(faces[i].x-tRuleOffsetRightUp.x,
    							faces[i].y-tRuleOffsetRightUp.y,
    							roiWidth,roiHeight));
    		}
    	}
    	t = (double)cvGetTickCount() - t;
    	// cout << "dt=" << (t>2.7e+08) << endl;
    	for (int i = 0; i < rois.size() && i < faces.size(); ++i)
    	{
    		int x=faces[i].x+faces[i].width/2.0-tRuleOffsetLeftUp.x;
    		int y=faces[i].y+faces[i].height/3.0 - tRuleOffsetLeftUp.y;
    		/// PID - регулятор для позиционирования камеры
 			dPrevX = distX;
 			distX = x-rois[i].x;
 			DeltaX = distX-dPrevX;
 			cntX += distX;
 			uX = distX*Kp + cntX*Ki + DeltaX*Kd;
	 		rois[i].x += uX;
 			dPrevY = distY;
	        distY = y-rois[i].y;
	        DeltaY = distY-dPrevY;
	        cntY += distY;
	        uY = distY*Kp + cntY*Ki + DeltaY*Kd;
        	rois[i].y += uY;
	        cout << "x=" << rois[i].x << " y=" << rois[i].y << endl;
	        if(rois[i].x<=0){
    			rois[i].x = 0;
    		}else if(maxCols < rois[i].x+rois[i].width){
	            rois[i].x = maxCols-rois[i].width;
	        }
	        if(rois[i].y <= 0){
	        	rois[i].y = 0;
	        }else if(maxRows < rois[i].y+rois[i].height){
	            rois[i].y=maxRows-rois[i].height;
	        }
    	}
    	// t = (double)cvGetTickCount();
    }
    
	bool detectMotion(Mat img){
		static Mat img_last,diff,gr, grLast,grDiff;
		static bool motionDetected=false;
        cvtColor( img, gr, COLOR_BGR2GRAY );
        if (!grLast.empty())
        {
            equalizeHist( gr, gr );
            GaussianBlur(gr, gr, Size(21,21), 0,0);
            // imshow("hist",gr);
            absdiff(gr, grLast, diff);
            // imshow("Diff",diff);
            threshold(diff, diff, 50, 255, cv::THRESH_BINARY);
            cout << (int)mean(diff)[0] << endl;
            motionDetected=((int)mean(diff)[0] > 0); // Движение в кадрике обнаружено
            imshow("threshold",diff);
        }
		grLast=gr.clone();
		return motionDetected;
	}       
int main( int argc, const char** argv )
{
    VideoCapture capture;
    Mat frame, image;
    const string scaleOpt = "--scale=";
    size_t scaleOptLen = scaleOpt.length();
    const string cascadeOpt = "--cascade=";
    size_t cascadeOptLen = cascadeOpt.length();
    const string nestedCascadeOpt = "--nested-cascade";
    size_t nestedCascadeOptLen = nestedCascadeOpt.length();
    const string tryFlipOpt = "--try-flip";
    size_t tryFlipOptLen = tryFlipOpt.length();
    string inputName;
    bool tryflip = false;

    help();

    CascadeClassifier cascade,cascade2, nestedCascade;
    double scale = 1;

    for( int i = 1; i < argc; i++ )
    {
        cout << "Processing " << i << " " <<  argv[i] << endl;
        if( cascadeOpt.compare( 0, cascadeOptLen, argv[i], cascadeOptLen ) == 0 )
        {
            cascadeName.assign( argv[i] + cascadeOptLen );
            cout << "  from which we have cascadeName= " << cascadeName << endl;
        }
        else if( nestedCascadeOpt.compare( 0, nestedCascadeOptLen, argv[i], nestedCascadeOptLen ) == 0 )
        {
            if( argv[i][nestedCascadeOpt.length()] == '=' )
                nestedCascadeName.assign( argv[i] + nestedCascadeOpt.length() + 1 );
            if( !nestedCascade.load( nestedCascadeName ) )
                cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
        }
        else if( scaleOpt.compare( 0, scaleOptLen, argv[i], scaleOptLen ) == 0 )
        {
            if( !sscanf( argv[i] + scaleOpt.length(), "%lf", &scale ) || scale < 1 )
                scale = 1;
            cout << " from which we read scale = " << scale << endl;
        }
        else if( tryFlipOpt.compare( 0, tryFlipOptLen, argv[i], tryFlipOptLen ) == 0 )
        {
            tryflip = true;
            cout << " will try to flip image horizontally to detect assymetric objects\n";
        }
        else if( argv[i][0] == '-' )
        {
            cerr << "WARNING: Unknown option %s" << argv[i] << endl;
        }
        else
            inputName.assign( argv[i] );
    }

    if( !cascade.load( cascadeName ) )
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        help();
        return -1;
    }

    if( !cascade2.load( cascade2Name ) )
    {
        cerr << "ERROR: Could not load classifier 2 cascade" << endl;
        help();
        return -1;
    }

    if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') )
    {
        int c = inputName.empty() ? 0 : inputName.c_str()[0] - '0' ;
        if(!capture.open(c))
            cout << "Capture from camera #" <<  c << " didn't work" << endl;
    }
    else if( inputName.size() )
    {
        image = imread( inputName, 1 );
        if( image.empty() )
        {
            if(!capture.open( inputName ))
                cout << "Could not read " << inputName << endl;
        }
    }
    else
    {
        image = imread( "../data/lena.jpg", 1 );
        if(image.empty()) cout << "Couldn't read ../data/lena.jpg" << endl;
    }
    Scalar thresholdSum=Scalar(50,200,200);
    if( capture.isOpened() )
    {
        cout << "Video capturing has been started ..." << endl;

        const int fourcc = CV_FOURCC('M', 'P', '4', '2'); // codecs
        const Size roiSize = Size((int) capture.get(CV_CAP_PROP_FRAME_WIDTH)/2,    // Acquire input size
                  (int) capture.get(CV_CAP_PROP_FRAME_HEIGHT)/2);
        const Size S = Size((int) capture.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) capture.get(CV_CAP_PROP_FRAME_HEIGHT));
        string outputName = "closeUp.avi";
        VideoWriter outputVideo(outputName, fourcc, capture.get(CAP_PROP_FPS), roiSize, true);  
        if (!outputVideo.isOpened())
        {
            cout  << "Could not open the output video for write: " << inputName  << endl;
            return -1;
        }

		Mat frameSmall, frame1;
		Mat gray;
		Mat motion;
        Mat smallImg;
		bool motionDetected=true;
		string title;
		std::string s;
		std::stringstream out;
		vector<Rect> rois;
		rois.push_back(Rect(S.width/2 - roiSize.width/2,  S.height/2-roiSize.height/2,
                            roiSize.width, roiSize.height));
		vector<Rect> faces;
        for(;;)
        {
            capture >> frame;
            if(frame.empty()) {
            	break;
            	cout << "Frame is empty" << endl;
            }
			frame1 = frame.clone();
			cvtColor( frame1, gray, COLOR_BGR2GRAY );
			static double fx = 1 / scale;   
            if(motionDetected){
			    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR );
			    equalizeHist( smallImg, smallImg );
			    
			    cascade.detectMultiScale( smallImg, faces,
			        1.1, 2, 0
			        |CASCADE_FIND_BIGGEST_OBJECT
			        |CASCADE_DO_ROUGH_SEARCH
			        |CASCADE_SCALE_IMAGE,
			        Size(30, 30) );
    			if(!faces.empty())rectangle(frame1,faces[0],Scalar(255,0,0), 1, 8, 0);
        	}
    		updateRoiCoords(faces,rois,capture.get(CV_CAP_PROP_FRAME_WIDTH),capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    		for(int i=0; i<rois.size(); i++){
    			/// Поиск движения в кадрике
    			motionDetected = detectMotion(frame(rois[i]));
    			rectangle(frame1,rois[i],Scalar(0,0,255), 3, 8, 0);
    		}             
    		
			// imshow("Basic window",frame1);    
            resize( frame1, frameSmall, Size(320,240), 0, 0, INTER_NEAREST );
            imshow("smallImg",frameSmall);    
            outputVideo << frame(rois[0]);
    		
    		int c = waitKey(10);
    		if( c == 27 || c == 'q' || c == 'Q' )break;
    
        }
    }
    else
    {
        cout << "Detecting face(s) in " << inputName << endl;
        if( !image.empty() )
        {
            detectAndDraw( image, cascade, nestedCascade, scale, tryflip );
            waitKey(0);
        }
        else if( !inputName.empty() )
        {
            /* assume it is a text file containing the
            list of the image filenames to be processed - one per line */
            FILE* f = fopen( inputName.c_str(), "rt" );
            if( f )
            {
                char buf[1000+1];
                while( fgets( buf, 1000, f ) )
                {
                    int len = (int)strlen(buf), c;
                    while( len > 0 && isspace(buf[len-1]) )
                        len--;
                    buf[len] = '\0';
                    cout << "file " << buf << endl;
                    image = imread( buf, 1 );
                    if( !image.empty() )
                    {
                        detectAndDraw( image, cascade, nestedCascade, scale, tryflip);
                        c = waitKey(0);
                        if( c == 27 || c == 'q' || c == 'Q' )
                            break;
                    }
                    else
                    {
                        cerr << "Aw snap, couldn't read image " << buf << endl;
                    }
                }
                fclose(f);
            }
        }
    }

    return 0;
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip)
{
	bool found_face=false;
    double t = 0;
    vector<Rect> faces, faces2;
    Mat gray, smallImg;
    static vector<Rect> facesBuf;
    const static Scalar colors[] =
    {
        Scalar(255, 0,      0),
        Scalar(255, 128,    0),
        Scalar(255, 255,    0),
        Scalar(0,   255,    0),
        Scalar(0,   128,    255),
        Scalar(0,   255,    255),
        Scalar(0,   0,      255),
        Scalar(255, 0,      255)
    };

    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale;
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );
    t = (double)cvGetTickCount();
    cascade.detectMultiScale( smallImg, faces,
        1.1, 2, 0
        // |CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        |CASCADE_SCALE_IMAGE,
        Size(30, 30) );
    if( tryflip )
    {
        flip(smallImg, smallImg, 1);
        cascade.detectMultiScale( smallImg, faces2,
                                 1.1, 2, 0
                                 // |CASCADE_FIND_BIGGEST_OBJECT
                                 //|CASCADE_DO_ROUGH_SEARCH
                                 |CASCADE_SCALE_IMAGE,
                                 Size(30, 30) );
        for( vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); r++ )
        {
            faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
        }
    }
    if(faces.size()>0){
    	found_face=true;
    	// facesBuf=faces;
    	// printf( "after flippping %d faces", faces.size() );
    }
    t = (double)cvGetTickCount() - t;
    printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    Rect roiRect;
    static Rect closeUpROI(0,0,320,240);
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Mat smallImgROI;
        Rect r = faces[i];
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;

        double aspect_ratio = (double)r.width/r.height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle( img, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)),
                       cvPoint(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                       color, 3, 8, 0);
        if( nestedCascade.empty() )
            continue;
        smallImgROI = smallImg( r );
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
            1.1, 2, 0
            //|CASCADE_FIND_BIGGEST_OBJECT
            //|CASCADE_DO_ROUGH_SEARCH
            //|CASCADE_DO_CANNY_PRUNING
            |CASCADE_SCALE_IMAGE,
            Size(30, 30) );
        for ( size_t j = 0; j < nestedObjects.size(); j++ )
        {
            Rect nr = nestedObjects[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
    }
    imshow( "result", img );   
	/*static Mat frameCloseUpLast;
	static Mat motionCloseUp;
	frameCloseUpLast = closeUpROI.clone();*/
	//Выводим крупный план в отдельное окно
    for ( size_t i = 0; i < facesBuf.size(); i++ ){
        if(facesBuf[i].x+closeUpROI.width < img.cols) {
            closeUpROI.x=facesBuf[i].x;
        }else{
            closeUpROI.x=img.cols-closeUpROI.width;
        }
        if(facesBuf[i].y+closeUpROI.height < img.rows){
            closeUpROI.y=facesBuf[i].y;
        }else{
            closeUpROI.y=img.rows-closeUpROI.height;
        }
        string title = "Face";
        std::string s;
		std::stringstream out;
		out << i;
		title += out.str();

        imshow( title, img(closeUpROI) );  
    }
    // return facesBuf;
}




