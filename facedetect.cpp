#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <opencv2/videoio/videoio.hpp>  // Video write
#include <ctime>

using namespace std;
using namespace cv;

static void help()
{
    cout << "Build date:" << __DATE__ << " " << __TIME__
            "During execution:\n\tHit any key to quit.\n"
            "\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip );

string cascadeFullName = "../../data/haarcascades/haarcascade_frontalface_alt.xml";
string cascadeProfName = "../../data/haarcascades/haarcascade_profileface.xml";
string cascadeLEyeName = "../../data/haarcascades/haarcascade_lefteye_2splits.xml";
string cascadeREyeName = "../../data/haarcascades/haarcascade_righteye_2splits.xml";

inline void scaleRect(Rect& r, const double& sc) {
    r.x *= sc;
    r.y *= sc;
    r.height *= sc;
    r.width *= sc;
}

void updateRoiCoords(const vector<Rect> &faces,
                     vector<Rect> &rois,
                     const int& maxCols,
                     const int& maxRows){
    static int roiHeight=240,roiWidth=roiHeight*4.0/3.0; //TODO 25.02.2016 Сделать для нескольких ROI с настраиваемыми размерами
    static Point leftUp(rois[0].width/3.0, rois[0].height/3.0);
    // static Point leftUp(0,0);
    static Point leftDown(rois[0].width/3.0, 2.0*rois[0].height/3.0);
    static Point rightUp(2.0*rois[0].width/3.0,rois[0].height/3.0);
    static Point rightDown(2.0*rois[0].width/3.0,2.0*rois[0].height/3.0);
    static Point center(rois[0].width/2,rois[0].height/3);
    static double t = 0;
    static float distX=0.0;
    static float distY=0.0;
    static float dPrevX=0.0;
    static float dPrevY=0.0;
    static float DeltaX=0.0;
    static float DeltaY=0.0;
    static double Kp=0.1, Ki=0.001, Kd=-0.09;//0.001 , 0.05
    static double cntX=0.0,cntY=0.0;
    static double uX,uY;

    //todo автоматическое увеличение количества кадриков
    if(rois.empty()){
        for (int i = 0; i < faces.size(); ++i)
        {
            rois.push_back(Rect((faces[i].x-rightUp.x),
                                (faces[i].y-rightUp.y),
                                roiWidth,roiHeight));
        }
    }
    Point target;
    for (int i = 0; i < rois.size() && i < faces.size(); ++i)
    {

        if(rois[i].width/3.0 - faces[i].width < 0 ) /// если лицо крупное, то держать его в центре кадра
            target = center;
        else if(faces[i].x+faces[i].width/2.0 < center.x
                && faces[i].x < rois[i].x+leftUp.x)
            target = leftUp;
        else if(faces[i].x+faces[i].width > rois[i].x+rightUp.x) // Камера посередине не будет реагировать
            target = rightUp;
        else
            target = center;

        int x=(faces[i].x+faces[i].width/2.0-target.x);
        int y=(faces[i].y+faces[i].height/3.0 - target.y);
        /// PID - регулятор для позиционирования камеры
        dPrevX = distX;
        distX  = x-rois[i].x;
        DeltaX = distX-dPrevX;
        cntX   += distX;
        uX     = distX*Kp + cntX*Ki + DeltaX*Kd;
        rois[i].x += uX;

        dPrevY = distY;
        distY  = y-rois[i].y;
        DeltaY = distY-dPrevY;
        cntY   += distY;
        uY     = distY*Kp + cntY*Ki + DeltaY*Kd;
        rois[i].y += uY;

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
}

void drawRects(Mat& img, const vector<Rect>& rects, string t="rect", Scalar color=Scalar(255,0,0),
              float fontScale=1.0, float textThickness=1.0, int textOffset=0, int thickness=1, int fontFace=CV_FONT_NORMAL){
    for (int i = 0; i < rects.size(); ++i)
    {
        stringstream title;
        title << t<<" "<< i;
        putText(img, title.str(),
                Point(rects[i].x,
                      rects[i].y-textOffset),
                fontFace, fontScale,color,textThickness);
        rectangle(img,rects[i],
                  color, thickness, 8, 0);
    }
}

inline void drawGoldenRules(Mat& img, const Rect& r,Scalar color=Scalar(0,255,0),const double& dotsRadius=1){
    //Отрисовка точек золотого сечения
    circle(img,Point(r.x + r.width/3.0,
                              r.y + r.height/3.0), 1,Scalar(0,255,0), dotsRadius);
    circle(img,Point(r.x + 2.0*r.width/3.0,
                              r.y + r.height/3.0),1,Scalar(0,255,0), dotsRadius);
    circle(img,Point(r.x + r.width/3.0,
                              r.y + 2.0*r.height/3.0),1,Scalar(0,255,0),dotsRadius);
    circle(img,Point(r.x + 2.0*r.width/3.0,
                              r.y + 2.0*r.height/3.0),1,Scalar(0,255,0),dotsRadius);
}

    int detectMotion(Mat img, int thresh=50, int blur=21, bool showPrev=false){
        static Mat diff,gr, grLast;
        int motion=0;
        cvtColor( img, gr, COLOR_BGR2GRAY );
        if (!grLast.empty())
        {
            equalizeHist( gr, gr );
            GaussianBlur(gr, gr, Size(blur,blur), 0,0);
            absdiff(gr, grLast, diff);
            threshold(diff, diff, thresh, 255, cv::THRESH_BINARY);
            motion=(int)mean(diff)[0]; // Движение в кадрике обнаружено

            if(showPrev){
                stringstream motion;
                motion<<"Motion "<<(int)mean(diff)[0];
                resize( diff, diff, Size(240*diff.cols/diff.rows,240), 0, 0, INTER_NEAREST );
                putText(diff, motion.str(),
                        Point(0,diff.rows),CV_FONT_NORMAL, 0.5, Scalar(255, 0,0));
                imshow("diff",diff);
            }
        }
		grLast=gr.clone();
        return motion;
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
    const string showPrevOpt =  "--show-preview";
    const string recPrevOpt = "--record-preview";
    const string roiSizeOpt = "--roiSize";
    size_t tryFlipOptLen = tryFlipOpt.length();
    string inputName;
    bool tryflip = false;
    bool showPreview = false;
    bool recordPreview = false;
    help();

    CascadeClassifier cascadeFull,cascadeProf, cascadeEyeL,cascadeEyeR; // Cascades for Full face and Profile face
    double scale = 1;

    for( int i = 1; i < argc; i++ )
    {
        cout << "Processing " << i << " " <<  argv[i] << endl;
        if( cascadeOpt.compare( 0, cascadeOptLen, argv[i], cascadeOptLen ) == 0 )
        {
            cascadeFullName.assign( argv[i] + cascadeOptLen );
            cout << "  from which we have cascadeName= " << cascadeFullName << endl;
        }
        else if( nestedCascadeOpt.compare( 0, nestedCascadeOptLen, argv[i], nestedCascadeOptLen ) == 0 )
        {
            cout << "nc" <<endl;
            if( argv[i][nestedCascadeOpt.length()] == '=' )
                cascadeLEyeName.assign( argv[i] + nestedCascadeOpt.length() + 1 );

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
            cerr << "WARNING: UnkoneIterEndn option %s" << argv[i] << endl;
        }
        else
            inputName.assign( argv[i] );
        if( string::npos!=showPrevOpt.find(argv[i]))
        {
            showPreview = true;
        }
        if( string::npos!=recPrevOpt.find(argv[i]))
        {
            recordPreview = true;
        }
    }

    if( !cascadeEyeL.load( cascadeLEyeName ) )
        cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
    if( !cascadeEyeR.load( cascadeREyeName ) )
        cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
    if( !cascadeFull.load( cascadeFullName ) )
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        help();
        return -1;
    }

    if( !cascadeProf.load( cascadeProfName ) )
    {
        cerr << "ERROR: Could not load classifier 2 cascade" << endl;
        help();
        return -1;
    }
    bool isWebcam=false;
    if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') )
    {
        isWebcam=true;
        int c = inputName.empty() ? 0 : inputName.c_str()[0] - '0' ;
        if(!capture.open(c))
            cout << "Capture from camera #" <<  c << " didn't work" << endl;
    }
    else if( inputName.size() )
    {
        isWebcam=false;
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
    if( capture.isOpened() )
    {
        cout << "Video capturing has been started ..." << endl;

        const int previewHeight = 480;
        const Size fullFrameSize = Size((int) capture.get(CV_CAP_PROP_FRAME_WIDTH),
                  (int) capture.get(CV_CAP_PROP_FRAME_HEIGHT));
        const float frameRatio= (float)fullFrameSize.width / (float)fullFrameSize.height;
        const Size roiSize = Size((int)(fullFrameSize.width/3.0),(int)(fullFrameSize.height/3.0));
        const Size previewSmallSize = Size((int)(previewHeight*frameRatio),previewHeight);
        const double fx = 1 / scale;
        const double ticksPerMsec=cvGetTickFrequency() * 1.0e3;
        const float thickness = 3.0*(float)fullFrameSize.height/(float)previewSmallSize.height;
        const int textOffset = thickness*2;
        const int textThickness = thickness/2;
        const int dotsRadius = thickness*2;
        const double fontScale = thickness/5.0;

        stringstream outFileTitle;

        if(isWebcam){
            time_t t = time(0);   // get time now
            struct tm * now = localtime( & t );
             outFileTitle << "webcam"
                          << (now->tm_year + 1900) << '_'
                          << (now->tm_mon + 1) << '_'
                          << now->tm_mday << "_"
                          << now->tm_hour <<"-"
                          << now->tm_min << "_"
                          << __DATE__ <<" "<< __TIME__ <<"_sc"<< scale;
        }else{
            outFileTitle << inputName.substr(inputName.find_last_of('/')+1)
            << __DATE__ <<" "<< __TIME__<<"_sc"<< scale;
        }
        int fps;
        int fourcc;
        if(isWebcam){
            fps = capture.get(CAP_PROP_FPS)/5;
            fourcc = VideoWriter::fourcc('M','J','P','G'); // codecs
        }
        else{
            fourcc = capture.get(CV_CAP_PROP_FOURCC); // codecs
            fps = capture.get(CAP_PROP_FPS);
        }
        long int videoLength = capture.get(CAP_PROP_FRAME_COUNT);
        long int frameCounter=1;
        VideoWriter outputVideo("results/closeUp_"+outFileTitle.str()+".avi",fourcc,
                                 fps, roiSize, true);
        if(!outputVideo.isOpened()){
            cout << "Could not open the output video ("
                 << "results/closeUp_"+outFileTitle.str()+".avi" <<") for writing"<<endl;
            return -1;
        }
        VideoWriter previewVideo;
        if(recordPreview){
             if(!previewVideo.open("results/test_"+outFileTitle.str()+".avi",fourcc,
                                   fps, previewSmallSize, true))
             {
                 cout << "Could not open the output video ("
                      << "results/test_"+outFileTitle.str()+".avi" <<") for writing"<<endl;
                 return -1;
             }
        }
        fstream logFile(("results/test_"+outFileTitle.str()+".csv").c_str(), fstream::out);
        if(!logFile.is_open()){
            cout << "Error with opening the file:" << "results/test_"+outFileTitle.str()+".csv" << endl;
        }else{
            logFile << "timestamp, ms\t"
                << "faceDetTime, ms\t"
                << "motDetTime, ms\t"
                << "updateTime, ms\t"
                << "oneIterTime, ms\t"
                << "faces[0].x, px\tfaces[0].y, px\t"
                << "rois[0].x, px\trois[0].y, px\t" << endl;
        }


        Mat previewSmall, previewFrame,gray,smallImg;
        Mat frameTemp;
        bool motionDetected=true;

        vector<Rect> rois;
        vector<Rect> facesFull,facesProf,eyesL,eyesR;
        vector<Rect> faceBuf;
        int64 oneIterEnd, oneIterStart,motDetStart,motDetEnd,faceDetStart,faceDetEnd,updateStart,updateEnd, timeStart;
        vector<int64> tStamps;
        vector<int> lines;
        double oneIterTime, motDetTime, faceDetTime,updateTime,timeEnd;
        string title = "Small preview";

        // точки золотого сечения по правилу третей
        Point leftUp;
        Point leftDown;
        Point rightUp;
        Point rightDown;

        capture >> frame;
        resize( frame, frameTemp,roiSize, 0, 0, INTER_LINEAR );
        if(showPreview) cvNamedWindow(title.c_str(), CV_WINDOW_NORMAL);

        try{
            tStamps.push_back(cvGetTickCount()), lines.push_back(__LINE__);
            while(faceBuf.empty() || faceBuf.size()<3){
                tStamps.push_back(cvGetTickCount()), lines.push_back(__LINE__);
                capture >> frame;
                if(frame.empty()) {clog << "Frame is empty" << endl; break;}

                tStamps.push_back(cvGetTickCount()), lines.push_back(__LINE__);

                cvtColor(frame, gray, COLOR_BGR2GRAY );
                resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR ); /// @todo 19.03.2016 сделать таймер

                tStamps.push_back(cvGetTickCount()), lines.push_back(__LINE__);

                cascadeFull.detectMultiScale( smallImg, facesFull,
                                              1.1, 2, 0
                                              |CASCADE_FIND_BIGGEST_OBJECT
                                              |CASCADE_DO_ROUGH_SEARCH
                                              |CASCADE_SCALE_IMAGE,Size(30, 30) );
                cascadeProf.detectMultiScale( smallImg, facesProf,
                                              1.1, 2, 0
                                              |CASCADE_FIND_BIGGEST_OBJECT
                                              |CASCADE_DO_ROUGH_SEARCH
                                              |CASCADE_SCALE_IMAGE, Size(30, 30)); ///@todo 19.03.2016 вывести все параметры отдельно

                tStamps.push_back(cvGetTickCount()), lines.push_back(__LINE__);

                if(!facesFull.empty()) faceBuf.push_back(facesFull[0]);
                if(!facesProf.empty()) faceBuf.push_back(facesProf[0]);

                resize( frame, frame,roiSize, 0, 0, INTER_LINEAR);
                outputVideo << frame; /// \todo 25.02.2016 сделать вывод для каждого лица

                tStamps.push_back(cvGetTickCount()), lines.push_back(__LINE__);

                if(recordPreview || showPreview)resize( frame, previewSmall,previewSmallSize, 0, 0, INTER_NEAREST);
                if(recordPreview)previewVideo << previewSmall;
                if(showPreview) imshow(title.c_str(), previewSmall);

                tStamps.push_back(cvGetTickCount()), lines.push_back(__LINE__);

                char c = waitKey(10);
                if(c==27)break;
                ++frameCounter;
                for(size_t i=0;i<tStamps.size()-1;++i){
                    cout << "lines " << lines[i] <<"-"<<lines[i+1]
                         << " t: " << ((double)(tStamps[i+1]-tStamps[i])/ticksPerMsec) << ", ms; " <<endl;
                }
                cout <<"frame:"<< frameCounter << " ("<<(int)((100*(float)frameCounter)/(float)videoLength) <<"% of video)"<< endl;
                tStamps.clear(), lines.clear();
            }

            cout << "! --- Faces found ---!" << endl;
            int x,y, sumX=0,sumY=0;
            for(size_t i; i<faceBuf.size();++i){
                sumX+=faceBuf[i].x;
                sumY+=faceBuf[i].y;
            }
            x=sumX/faceBuf.size();
            y=sumY/faceBuf.size();

            rois.push_back(Rect(scale*(x-rightUp.x),
                                scale*(y-rightUp.y),
                                roiSize.width,roiSize.height));
            if(rois[0].x<=0){
                rois[0].x = 0;
            }else if(frame.cols < rois[0].x+rois[0].width){
                rois[0].x = frame.cols-rois[0].width;
            }
            if(rois[0].y <= 0){
                rois[0].y = 0;
            }else if(frame.rows < rois[0].y+rois[0].height){
                rois[0].y=frame.rows-rois[0].height;
            }
        }
        catch (cv::Exception& e){
            clog << "Problem in line " << e.line << endl;
        }

        timeStart = cvGetTickCount();
Mat result;
  ////////////////////////////////////////////////
        for(;;)
        {
            oneIterStart = cvGetTickCount(); 
            capture >> frame;
            cout <<"frame:"<< ++frameCounter << " ("<<(int)((100*(float)frameCounter)/(float)videoLength) <<"% of video)"<< endl;
            if(frame.empty()) {
                clog << "Frame is empty" << endl;
                break;
            }
            previewFrame = frame.clone();

            faceDetStart = cvGetTickCount();
            if(motionDetected){
                /// @todo 26.03.2016 поиск лиц только в области интереса.
                cvtColor( frame, gray, COLOR_BGR2GRAY );
                resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR );
                /* Поиск лиц в анфас */
                cascadeFull.detectMultiScale( smallImg, facesFull,
                    1.1, 2, 0
//                    |CASCADE_FIND_BIGGEST_OBJECT
//                    |CASCADE_DO_ROUGH_SEARCH
                    |CASCADE_SCALE_IMAGE,
                    Size(30, 30) );

                /// Поиск лиц в профиль
                cascadeProf.detectMultiScale( smallImg, facesProf,
                    1.1, 2, 0
//                    |CASCADE_FIND_BIGGEST_OBJECT
//                    |CASCADE_DO_ROUGH_SEARCH
                    |CASCADE_SCALE_IMAGE,
                    Size(30, 30));
                for (size_t i=0; i<facesFull.size(); ++i) {
                    scaleRect(facesFull[i],scale);
                }
                for (size_t i=0; i<facesProf.size(); ++i) {
                    scaleRect(facesProf[i],scale);
                }
                //Отрисовка распознаных объектов на превью
                if(showPreview || recordPreview){
                    drawRects(previewFrame,facesFull,"Full face",Scalar(255,0,0));
                    drawRects(previewFrame,facesProf,"Profile",Scalar(255,127,0));
                }
            }


            faceDetEnd = cvGetTickCount();

            updateStart = cvGetTickCount();
            updateRoiCoords(facesFull,rois,fullFrameSize.width,fullFrameSize.height);
            updateRoiCoords(facesProf,rois,fullFrameSize.width,fullFrameSize.height);
            updateEnd = cvGetTickCount();

            motDetStart = cvGetTickCount();
            for (int i = 0; i < rois.size(); ++i)
                motionDetected = (detectMotion(frame(rois[i]),50,21,showPreview)>0);
            motDetEnd = cvGetTickCount();

            resize(frame(rois[0]), result , roiSize, 0,0, INTER_LINEAR );
            outputVideo << result ; /// \todo 25.02.2016 сделать вывод для каждого лица

            if(showPreview || recordPreview){ // Отрисовка области интереса
                drawRects(previewFrame,rois,"ROI",Scalar(0,0,255),fontScale,textThickness,textOffset);
                for (int i = 0; i < rois.size(); ++i)
                    drawGoldenRules(previewFrame,rois[i],Scalar(0,255,0),dotsRadius);

                resize( previewFrame, previewSmall, previewSmallSize, 0, 0, INTER_NEAREST );
                if(recordPreview)
                    previewVideo << previewSmall;
                if(showPreview)
                    imshow(title.c_str(),previewSmall);
            }

            int c = waitKey(10);
            if( c == 27 || c == 'q' || c == 'Q' )break;
    
            /// Запись статистики
            oneIterEnd = cvGetTickCount(); 
            timeEnd = (double)(cvGetTickCount() - timeStart)/ticksPerMsec;
            faceDetTime = (double)(faceDetEnd - faceDetStart)/ticksPerMsec;
            updateTime =  (double)(updateEnd - updateStart)/ticksPerMsec;
            motDetTime  = (double)(motDetEnd - motDetStart)/ticksPerMsec;
            oneIterTime = (double)(oneIterEnd - oneIterStart)/ticksPerMsec;
            if(logFile.is_open()) {
                logFile  << timeEnd << "\t"
                    << faceDetTime << "\t" 
                    << motDetTime << "\t" 
                    << updateTime << "\t"
                    << oneIterTime << "\t";
                if(!facesFull.empty())
                    logFile << facesFull[0].x << "\t"
                                     << facesFull[0].y << "\t"; else logFile << "\t\t";
                if(!rois.empty())
                    logFile << rois[0].x << "\t"
                                    << rois[0].y << "\t";else logFile << "\t\t";
                logFile  << endl;
            }
            oneIterEnd = faceDetEnd = motDetEnd = -1;
        }
        if(logFile.is_open())logFile.close();
        cout << "The results have been written to " << "''"+outFileTitle.str()+"''" << endl;
        cvDestroyAllWindows();
    }
    else
    {
        cout << "Detecting face(s) in " << inputName << endl;
        if( !image.empty() )
        {
            detectAndDraw( image, cascadeFull, cascadeEyeL, scale, tryflip );
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
                        detectAndDraw( image, cascadeFull, cascadeEyeL, scale, tryflip);
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
        nestedCascade.detectMultiScale(smallImgROI, nestedObjects,
            1.1, 2, 0
//            |CASCADE_FIND_BIGGEST_OBJECT
            |CASCADE_DO_ROUGH_SEARCH
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




