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

void CvWindow::sendFrame(Frame *frame)
{
    if(frame->cameOnTime() && !frame->getPixels().empty()){
        que.push(*frame);
        imshow(winname,que.front().getPixels());
        que.pop();
        waitKey(1);
    }
}
