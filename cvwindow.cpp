#include "cvwindow.h"

CvWindow::CvWindow(const string _winname, int _flags)
    : winname(_winname)
{
    std::clog<< "CvWindow created " << winname << std::endl;
    namedWindow(winname,_flags);
}

CvWindow::~CvWindow()
{
    destroyWindow(winname);
}

void CvWindow::sendFrame(Mat &frame) const
{
//    static bool b=true;
    imshow(winname,frame);
    waitKey(1);
}
