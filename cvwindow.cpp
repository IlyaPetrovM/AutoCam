#include "cvwindow.h"

CvWindow::CvWindow(int winWidth, int winHeight, const string _winname, int _flags)
    : Output(winWidth,winHeight), winname(_winname)
{
    std::clog<< "CvWindow created " << winname << std::endl;
    namedWindow(winname,_flags);
    resizeWindow(winname,targetWidth,targetHeight);
}

CvWindow::~CvWindow()
{
    destroyWindow(winname);
}

void CvWindow::sendFrame(Frame *frame)
{
    if(frame->cameOnTime() && !frame->getPixels().empty()){
        static Mat tmp;
        resize(frame->getPixels(),tmp,Size(targetWidth,targetHeight)); //todo target width and height
        imshow(winname,tmp);
        waitKey(20);
    }
}
