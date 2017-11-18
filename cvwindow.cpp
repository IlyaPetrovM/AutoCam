#include "cvwindow.h"

CvWindow::CvWindow(int winWidth, int winHeight, const string _winname, int _flags)
    : Output(winWidth,winHeight), winname(_winname)
{
    std::clog<< "CvWindow created " << winname << std::endl;
    namedWindow(winname,_flags);
    resizeWindow(winname,winWidth,winHeight);
}

CvWindow::~CvWindow()
{
    destroyWindow(winname);
}

void CvWindow::sendFrame(const Mat &frame)
{
    if(!frame.empty()){
        imshow(winname,frame);
        waitKey(1);
    }
}
